#ifndef RENDERER_H
#define RENDERER_H

#include "device.h"
#include "swapchain.h"
#include "utils/color.h"

typedef struct {
  VkRenderPass renderPass;
  VkFramebuffer *framebuffers;
  VkCommandPool cmdPool;
  VkCommandBuffer cmdBuffer;
  VkSemaphore imageAvailable;
  VkSemaphore renderFinished;
  VkFence inFlight;
  VkClearValue clearColor;
} VsRenderer;

VsRenderer *vs_renderer_create(VsDevice *device, VsSwapchain *swapchain);

void vs_renderer_destroy(VsRenderer *renderer, VsDevice *device,
                         VsSwapchain *swapchain);

void vs_renderer_set_clear_color(VsRenderer *renderer, VsColor color);
void vs_renderer_draw(VsRenderer *renderer, VsDevice *device,
                      VsSwapchain *swapchain);

#endif // RENDERER_H
