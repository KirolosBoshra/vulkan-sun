#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>

VsRenderer *vs_renderer_create(VsDevice *device, VsSwapchain *swapchain) {
  VsRenderer *renderer = malloc(sizeof(VsRenderer));
  if (!renderer) {
    fprintf(stderr, "failed to allocate renderer\n");
    return NULL;
  }

  renderer->clearColor = vs_color_clear_value(VS_COLOR_SKYBLUE);

  VkAttachmentDescription colorAttachment = {
      .format = swapchain->format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkAttachmentReference colorRef = {
      .attachment = 0,
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  VkSubpassDescription subpass = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorRef,
  };

  VkSubpassDependency dependency = {
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  VkRenderPassCreateInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &colorAttachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
  };

  if (vkCreateRenderPass(device->device, &renderPassInfo, NULL,
                         &renderer->renderPass) != VK_SUCCESS) {
    fprintf(stderr, "failed to create render pass\n");
    free(renderer);
    return NULL;
  }

  renderer->framebuffers = malloc(swapchain->count * sizeof(VkFramebuffer));
  for (uint32_t i = 0; i < swapchain->count; i++) {
    VkFramebufferCreateInfo fbInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderer->renderPass,
        .attachmentCount = 1,
        .pAttachments = &swapchain->views[i],
        .width = swapchain->extent.width,
        .height = swapchain->extent.height,
        .layers = 1,
    };
    if (vkCreateFramebuffer(device->device, &fbInfo, NULL,
                            &renderer->framebuffers[i]) != VK_SUCCESS) {
      fprintf(stderr, "failed to create framebuffer\n");
      for (uint32_t j = 0; j < i; j++)
        vkDestroyFramebuffer(device->device, renderer->framebuffers[j], NULL);
      vkDestroyRenderPass(device->device, renderer->renderPass, NULL);
      free(renderer->framebuffers);
      free(renderer);
      return NULL;
    }
  }

  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = device->graphicsFamily,
  };

  if (vkCreateCommandPool(device->device, &poolInfo, NULL,
                          &renderer->cmdPool) != VK_SUCCESS) {
    fprintf(stderr, "failed to create command pool\n");
    for (uint32_t i = 0; i < swapchain->count; i++)
      vkDestroyFramebuffer(device->device, renderer->framebuffers[i], NULL);
    vkDestroyRenderPass(device->device, renderer->renderPass, NULL);
    free(renderer->framebuffers);
    free(renderer);
    return NULL;
  }

  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = renderer->cmdPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  if (vkAllocateCommandBuffers(device->device, &allocInfo,
                               &renderer->cmdBuffer) != VK_SUCCESS) {
    fprintf(stderr, "failed to allocate command buffer\n");
    vkDestroyCommandPool(device->device, renderer->cmdPool, NULL);
    for (uint32_t i = 0; i < swapchain->count; i++)
      vkDestroyFramebuffer(device->device, renderer->framebuffers[i], NULL);
    vkDestroyRenderPass(device->device, renderer->renderPass, NULL);
    free(renderer->framebuffers);
    free(renderer);
    return NULL;
  }

  VkSemaphoreCreateInfo semInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  if (vkCreateSemaphore(device->device, &semInfo, NULL,
                        &renderer->imageAvailable) != VK_SUCCESS ||
      vkCreateSemaphore(device->device, &semInfo, NULL,
                        &renderer->renderFinished) != VK_SUCCESS) {
    fprintf(stderr, "failed to create semaphores\n");
    vkDestroyCommandPool(device->device, renderer->cmdPool, NULL);
    for (uint32_t i = 0; i < swapchain->count; i++)
      vkDestroyFramebuffer(device->device, renderer->framebuffers[i], NULL);
    vkDestroyRenderPass(device->device, renderer->renderPass, NULL);
    free(renderer->framebuffers);
    free(renderer);
    return NULL;
  }

  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  if (vkCreateFence(device->device, &fenceInfo, NULL, &renderer->inFlight) !=
      VK_SUCCESS) {
    fprintf(stderr, "failed to create fence\n");
    vkDestroySemaphore(device->device, renderer->imageAvailable, NULL);
    vkDestroySemaphore(device->device, renderer->renderFinished, NULL);
    vkDestroyCommandPool(device->device, renderer->cmdPool, NULL);
    for (uint32_t i = 0; i < swapchain->count; i++)
      vkDestroyFramebuffer(device->device, renderer->framebuffers[i], NULL);
    vkDestroyRenderPass(device->device, renderer->renderPass, NULL);
    free(renderer->framebuffers);
    free(renderer);
    return NULL;
  }

  return renderer;
}

void vs_renderer_destroy(VsRenderer *renderer, VsDevice *device,
                         VsSwapchain *swapchain) {
  if (!renderer)
    return;

  vkDestroyFence(device->device, renderer->inFlight, NULL);
  vkDestroySemaphore(device->device, renderer->imageAvailable, NULL);
  vkDestroySemaphore(device->device, renderer->renderFinished, NULL);
  vkDestroyCommandPool(device->device, renderer->cmdPool, NULL);

  for (uint32_t i = 0; i < swapchain->count; i++)
    vkDestroyFramebuffer(device->device, renderer->framebuffers[i], NULL);

  vkDestroyRenderPass(device->device, renderer->renderPass, NULL);

  free(renderer->framebuffers);
  free(renderer);
}

void vs_renderer_set_clear_color(VsRenderer *renderer, VsColor color) {
  if (!renderer)
    return;
  renderer->clearColor = vs_color_clear_value(color);
}

void vs_renderer_draw(VsRenderer *renderer, VsDevice *device,
                      VsSwapchain *swapchain) {
  vkWaitForFences(device->device, 1, &renderer->inFlight, VK_TRUE, UINT64_MAX);
  vkResetFences(device->device, 1, &renderer->inFlight);

  uint32_t imageIndex = 0;
  vkAcquireNextImageKHR(device->device, swapchain->handle, UINT64_MAX,
                        renderer->imageAvailable, VK_NULL_HANDLE, &imageIndex);

  vkResetCommandBuffer(renderer->cmdBuffer, 0);

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(renderer->cmdBuffer, &beginInfo);

  VkRenderPassBeginInfo rpBegin = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = renderer->renderPass,
      .framebuffer = renderer->framebuffers[imageIndex],
      .renderArea = {.offset = {0, 0}, .extent = swapchain->extent},
      .clearValueCount = 1,
      .pClearValues = &renderer->clearColor,
  };
  vkCmdBeginRenderPass(renderer->cmdBuffer, &rpBegin,
                       VK_SUBPASS_CONTENTS_INLINE);
  vkCmdEndRenderPass(renderer->cmdBuffer);

  vkEndCommandBuffer(renderer->cmdBuffer);

  VkPipelineStageFlags waitStage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &renderer->imageAvailable,
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &renderer->cmdBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &renderer->renderFinished,
  };

  vkQueueSubmit(device->qQueue, 1, &submitInfo, renderer->inFlight);

  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &renderer->renderFinished,
      .swapchainCount = 1,
      .pSwapchains = &swapchain->handle,
      .pImageIndices = &imageIndex,
  };

  vkQueuePresentKHR(device->pQueue, &presentInfo);
}
