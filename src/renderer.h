#ifndef RENDERER_H
#define RENDERER_H
#include "utils/globals.h"

#include "device.h"
#include "mesh.h"
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
  u32 currentFrame;
  u32 imageIndex;
  u8 pushData[128];
} VsRenderer;

VsRenderer *vs_renderer_create(VsDevice *device);
void vs_renderer_destroy(VsRenderer *renderer, VsDevice *device);

void vs_renderer_set_clear_color(VsRenderer *renderer, VsColor color);

i32 vs_renderer_frame_begin(VsRenderer *renderer, VsDevice *device,
                            VsSwapchain *swapchain);
void vs_renderer_draw(VsRenderer *renderer, VsPipeline *pipeline,
                      VkExtent2D extent);
void vs_renderer_draw_mesh(VsRenderer *renderer, VsPipeline *pipeline,
                            Mesh *mesh, VkExtent2D extent);
i32 vs_renderer_frame_end(VsRenderer *renderer, VsDevice *device,
                          VsSwapchain *swapchain);

#endif
