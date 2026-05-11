#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>

// Transition a swapchain image between layouts using the Vulkan 1.3
static void image_barrier(VkCommandBuffer cmd, VkImage image,
                          VkImageLayout from, VkImageLayout to) {
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
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
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

VsRenderer *vs_renderer_create(VsDevice *device) {
  VsRenderer *r = malloc(sizeof *r);
  if (!r) {
    fprintf(stderr, "failed to allocate renderer\n");
    return NULL;
  }

  r->clearColor = vs_color_clear_value(VS_COLOR_SKYBLUE);
  r->currentFrame = 0;

  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = device->graphicsFamily,
  };
  if (vkCreateCommandPool(device->device, &poolInfo, NULL, &r->cmdPool) !=
      VK_SUCCESS) {
    fprintf(stderr, "failed to create command pool\n");
    free(r);
    return NULL;
  }

  // One command buffer per frame-in-flight
  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = r->cmdPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = VS_MAX_FRAMES_IN_FLIGHT,
  };
  if (vkAllocateCommandBuffers(device->device, &allocInfo, r->cmdBufs) !=
      VK_SUCCESS) {
    fprintf(stderr, "failed to allocate command buffers\n");
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

  for (int i = 0; i < VS_MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device->device, &semInfo, NULL,
                          &r->imageAvailable[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device->device, &semInfo, NULL,
                          &r->renderFinished[i]) != VK_SUCCESS ||
        vkCreateFence(device->device, &fenceInfo, NULL, &r->inFlight[i]) !=
            VK_SUCCESS) {
      fprintf(stderr, "failed to create sync objects for frame %d\n", i);
      for (int j = 0; j < i; j++) {
        vkDestroyFence(device->device, r->inFlight[j], NULL);
        vkDestroySemaphore(device->device, r->renderFinished[j], NULL);
        vkDestroySemaphore(device->device, r->imageAvailable[j], NULL);
      }
      vkDestroyCommandPool(device->device, r->cmdPool, NULL);
      free(r);
      return NULL;
    }
  }

  return r;
}

void vs_renderer_destroy(VsRenderer *renderer, VsDevice *device) {
  if (!renderer)
    return;
  for (int i = 0; i < VS_MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyFence(device->device, renderer->inFlight[i], NULL);
    vkDestroySemaphore(device->device, renderer->renderFinished[i], NULL);
    vkDestroySemaphore(device->device, renderer->imageAvailable[i], NULL);
  }
  vkDestroyCommandPool(device->device, renderer->cmdPool, NULL);
  free(renderer);
}

void vs_renderer_set_clear_color(VsRenderer *renderer, VsColor color) {
  if (!renderer)
    return;
  renderer->clearColor = vs_color_clear_value(color);
}

int vs_renderer_draw(VsRenderer *renderer, VsDevice *device,
                     VsSwapchain *swapchain) {
  uint32_t frame = renderer->currentFrame;

  vkWaitForFences(device->device, 1, &renderer->inFlight[frame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      device->device, swapchain->handle, UINT64_MAX,
      renderer->imageAvailable[frame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return 1;
  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    fprintf(stderr, "vkAcquireNextImageKHR failed\n");
    return -1;
  }

  vkResetFences(device->device, 1, &renderer->inFlight[frame]);

  VkCommandBuffer cmd = renderer->cmdBufs[frame];
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(cmd, &beginInfo);

  image_barrier(cmd, swapchain->images[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  // Begin dynamic rendering
  VkRenderingAttachmentInfo colorAtt = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = swapchain->views[imageIndex],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = renderer->clearColor,
  };
  VkRenderingInfo renderInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {.extent = swapchain->extent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAtt,
  };

  vkCmdBeginRendering(cmd, &renderInfo);
  vkCmdEndRendering(cmd);

  image_barrier(cmd, swapchain->images[imageIndex],
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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
    fprintf(stderr, "vkQueueSubmit failed\n");
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
  result = vkQueuePresentKHR(device->pQueue, &presentInfo);

  renderer->currentFrame = (frame + 1) % VS_MAX_FRAMES_IN_FLIGHT;

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    return 1;
  if (result != VK_SUCCESS) {
    fprintf(stderr, "vkQueuePresentKHR failed\n");
    return -1;
  }
  return 0;
}
