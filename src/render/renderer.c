#include "renderer.h"
#include "descriptor.h"
#include "core/logger.h"
#include "utils/globals.h"
#include "types/types.h"
#include <stdlib.h>
#include <string.h>

static void image_barrier(VkCommandBuffer cmd, VkImage image,
                          VkImageLayout from, VkImageLayout to,
                          VkImageAspectFlags aspect) {
  VkImageMemoryBarrier2 b = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .dstAccessMask =
          VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
      .oldLayout = from,
      .newLayout = to,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = aspect,
              .levelCount = 1,
              .layerCount = 1,
          },
  };
  VkDependencyInfo dep = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &b,
  };
  vkCmdPipelineBarrier2(cmd, &dep);
}

static void set_viewport_scissor(VkCommandBuffer cmd, VkExtent2D extent) {
  VkViewport vp = {0, 0, (f32)extent.width, (f32)extent.height, 0, 1};
  VkRect2D scissor = {{0, 0}, extent};
  vkCmdSetViewport(cmd, 0, 1, &vp);
  vkCmdSetScissor(cmd, 0, 1, &scissor);
}

Renderer *renderer_create(Device *device, u32 uboSize) {
  log_trace("creating renderer");

  Renderer *r = malloc(sizeof *r);
  if (!r) {
    log_error("failed to allocate renderer");
    return NULL;
  }

  r->clearColor = color_clear_value(COLOR_SKYBLUE);
  r->currentFrame = 0;

  DescriptorAllocator *descAlloc = &r->descAlloc;
  PoolSizeRatio ratios[] = {
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 0.5f}};

  desc_alloc_init(descAlloc, 8, ratios, sizeof(ratios) / sizeof(*ratios));
  desc_alloc_init(&r->transientAlloc, 8, ratios,
                     sizeof(ratios) / sizeof(*ratios));

  DescriptorLayoutBuilder layoutBuilder;
  desc_layout_init(&layoutBuilder);
  desc_layout_add(&layoutBuilder, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                     VK_SHADER_STAGE_VERTEX_BIT);
  r->sceneLayout = desc_layout_build(&layoutBuilder, device->device);
  desc_layout_cleanup(&layoutBuilder);

  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = device->graphicsFamily,
  };
  if (vkCreateCommandPool(device->device, &poolInfo, NULL, &r->cmdPool) !=
      VK_SUCCESS) {
    log_error("failed to create command pool");
    free(r);
    return NULL;
  }

  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = r->cmdPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
  };
  if (vkAllocateCommandBuffers(device->device, &allocInfo, r->cmdBufs) !=
      VK_SUCCESS) {
    log_error("failed to allocate command buffers");
    vkDestroyCommandPool(device->device, r->cmdPool, NULL);
    free(r);
    return NULL;
  }

  VkSemaphoreCreateInfo semInfo = {.sType =
                                       VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  for (i32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device->device, &semInfo, NULL,
                          &r->imageAvailable[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device->device, &semInfo, NULL,
                          &r->renderFinished[i]) != VK_SUCCESS ||
        vkCreateFence(device->device, &fenceInfo, NULL, &r->inFlight[i]) !=
            VK_SUCCESS) {
      log_error("failed to create sync objects for frame %d", i);
      for (i32 j = 0; j < i; j++) {
        vkDestroyFence(device->device, r->inFlight[j], NULL);
        vkDestroySemaphore(device->device, r->renderFinished[j], NULL);
        vkDestroySemaphore(device->device, r->imageAvailable[j], NULL);
      }
      vkDestroyCommandPool(device->device, r->cmdPool, NULL);
      free(r);
      return NULL;
    }
    r->uboBuffers[i] = create_buffer(
        sizeof(Mat4), device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkMapMemory(device->device, r->uboBuffers[i].memory, 0, sizeof(Mat4), 0,
                &r->uboMapped[i]);
    r->sceneSets[i] =
        desc_alloc_allocate(descAlloc, device->device, r->sceneLayout);
    VkWriteDescriptorSet writes[1];
    VkDescriptorBufferInfo bufInfos[1];
    VkDescriptorImageInfo imgInfos[1];
    DescriptorWriter w;
    desc_writer_init(&w, bufInfos, imgInfos, writes, 1);
    desc_writer_write_buffer(&w, 0, r->uboBuffers[i].buffer, sizeof(Mat4), 0,
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    desc_writer_update(&w, device->device, r->sceneSets[i]);
  }

  r->uboSize = uboSize;

  log_trace("renderer created");
  return r;
}

void renderer_destroy(Renderer *renderer, Device *device) {
  log_trace("destroying renderer");
  if (!renderer)
    return;
  for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyFence(device->device, renderer->inFlight[i], NULL);
    vkDestroySemaphore(device->device, renderer->renderFinished[i], NULL);
    vkDestroySemaphore(device->device, renderer->imageAvailable[i], NULL);
    vkUnmapMemory(device->device, renderer->uboBuffers[i].memory);
    destroy_buffer(&renderer->uboBuffers[i], device->device);
  }
  vkDestroyCommandPool(device->device, renderer->cmdPool, NULL);
  desc_alloc_destroy(&renderer->transientAlloc, device->device);
  desc_alloc_destroy(&renderer->descAlloc, device->device);
  free(renderer);
}

void renderer_set_clear_color(Renderer *renderer, Color color) {
  if (!renderer)
    return;
  renderer->clearColor = color_clear_value(color);
}

i32 renderer_frame_begin(Renderer *renderer, Device *device,
                            Swapchain *swapchain, VkClearValue clearColor) {
  u32 frame = renderer->currentFrame;

  vkWaitForFences(device->device, 1, &renderer->inFlight[frame], VK_TRUE,
                  UINT64_MAX);

  u32 imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      device->device, swapchain->handle, UINT64_MAX,
      renderer->imageAvailable[frame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return 1;
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    log_error("vkAcquireNextImageKHR failed");
    return -1;
  }

  renderer->imageIndex = imageIndex;

  vkResetFences(device->device, 1, &renderer->inFlight[frame]);

  VkCommandBuffer cmd = renderer->cmdBufs[frame];
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(cmd, &beginInfo);

  image_barrier(cmd, swapchain->images[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT);

  image_barrier(cmd, swapchain->depth.image, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_DEPTH_BIT);

  VkClearValue depthClear = {.depthStencil = {.depth = 1.0f}};

  VkRenderingAttachmentInfo colorAtt = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = swapchain->views[imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = clearColor,
  };
  VkRenderingAttachmentInfo depthAtt = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = swapchain->depth.view,
      .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .clearValue = depthClear,
  };
  VkRenderingInfo renderInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {.extent = swapchain->extent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAtt,
      .pDepthAttachment = &depthAtt,
  };

  vkCmdBeginRendering(cmd, &renderInfo);

  return 0;
}

void renderer_push_constants(Renderer *renderer, Pipeline *pipeline,
                                void *data) {
  u32 frame = renderer->currentFrame;
  VkCommandBuffer cmd = renderer->cmdBufs[frame];
  vkCmdPushConstants(cmd, pipeline->layout, pipeline->pushStages, 0,
                     pipeline->pushSize, data);
}

void renderer_draw(Renderer *renderer, Pipeline *pipeline, VkExtent2D extent) {
  u32 frame = renderer->currentFrame;
  VkCommandBuffer cmd = renderer->cmdBufs[frame];

  set_viewport_scissor(cmd, extent);
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
  vkCmdDraw(cmd, 3, 1, 0, 0);
}

void renderer_draw_mesh(Renderer *renderer, Pipeline *pipeline, Mesh *mesh,
                           VkExtent2D extent) {
  u32 frame = renderer->currentFrame;
  VkCommandBuffer cmd = renderer->cmdBufs[frame];
  VkDeviceSize offset = 0;

  set_viewport_scissor(cmd, extent);
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline->layout, 0, 1, &renderer->sceneSets[frame],
                          0, NULL);
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
  vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(cmd, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(cmd, mesh->indexCount, 1, 0, 0, 0);
}

i32 renderer_frame_end(Renderer *renderer, Device *device,
                          Swapchain *swapchain) {
  u32 frame = renderer->currentFrame;
  VkCommandBuffer cmd = renderer->cmdBufs[frame];
  u32 imageIndex = renderer->imageIndex;

  vkCmdEndRendering(cmd);

  image_barrier(cmd, swapchain->images[imageIndex],
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);

  vkEndCommandBuffer(cmd);

  VkPipelineStageFlags waitStage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &renderer->imageAvailable[frame],
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &renderer->renderFinished[frame],
  };
  if (vkQueueSubmit(device->qQueue, 1, &submitInfo,
                    renderer->inFlight[frame]) != VK_SUCCESS) {
    log_error("vkQueueSubmit failed");
    return -1;
  }

  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &renderer->renderFinished[frame],
      .swapchainCount = 1,
      .pSwapchains = &swapchain->handle,
      .pImageIndices = &imageIndex,
  };
  VkResult result = vkQueuePresentKHR(device->pQueue, &presentInfo);

  renderer->currentFrame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    return 1;
  if (result != VK_SUCCESS) {
    log_error("vkQueuePresentKHR failed");
    return -1;
  }
  return 0;
}

void renderer_update_ubo(Renderer *renderer, void *data) {
  u32 frame = renderer->currentFrame;
  memcpy(renderer->uboMapped[frame], data, renderer->uboSize);
}
