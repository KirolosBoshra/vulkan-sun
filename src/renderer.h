#ifndef RENDERER_H
#define RENDERER_H

#include "device.h"
#include "pipline.h"
#include "swapchain.h"
#include "utils/color.h"

#define VS_MAX_FRAMES_IN_FLIGHT 2

typedef struct {
  VkCommandPool cmdPool;
  VkCommandBuffer cmdBufs[VS_MAX_FRAMES_IN_FLIGHT];
  VkSemaphore imageAvailable[VS_MAX_FRAMES_IN_FLIGHT];
  VkSemaphore renderFinished[VS_MAX_FRAMES_IN_FLIGHT];
  VkFence inFlight[VS_MAX_FRAMES_IN_FLIGHT];
  VkClearValue clearColor;
  uint32_t currentFrame;
  VsPipeline *pipeline;
} VsRenderer;

VsRenderer *vs_renderer_create(VsDevice *device);

void vs_renderer_destroy(VsRenderer *renderer, VsDevice *device);

void vs_renderer_set_clear_color(VsRenderer *renderer, VsColor color);
int vs_renderer_create_pipeline(VsRenderer *renderer, VsDevice *device,
                                VkFormat colorFormat,
                                const char *vertPath, const char *fragPath);
int vs_renderer_draw(VsRenderer *renderer, VsDevice *device,
                     VsSwapchain *swapchain);

#endif // RENDERER_H
