#ifndef RENDERER_H
#define RENDERER_H
#include "utils/globals.h"

#include "descriptor.h"
#include "device.h"
#include "types/mesh.h"
#include "pipeline.h"
#include "swapchain.h"
#include "utils/color.h"
#include "types/types.h"

#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
  VkCommandPool cmdPool;
  VkCommandBuffer cmdBufs[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
  VkFence inFlight[MAX_FRAMES_IN_FLIGHT];
  DescriptorAllocator descAlloc;
  DescriptorAllocator transientAlloc;
  VkDescriptorSetLayout sceneLayout;
  VkDescriptorSet sceneSets[MAX_FRAMES_IN_FLIGHT];
  Buffer uboBuffers[MAX_FRAMES_IN_FLIGHT];
  void *uboMapped[MAX_FRAMES_IN_FLIGHT];
  VkClearValue clearColor;
  u32 uboSize;
  u32 currentFrame;
  u32 imageIndex;
} Renderer;

Renderer *renderer_create(Device *device, u32 uboSize);
void renderer_destroy(Renderer *renderer, Device *device);

void renderer_set_clear_color(Renderer *renderer, Color color);

i32 renderer_frame_begin(Renderer *renderer, Device *device,
                            Swapchain *swapchain, VkClearValue clearColor);
void renderer_push_constants(Renderer *renderer, Pipeline *pipeline, void *data);
void renderer_draw(Renderer *renderer, Pipeline *pipeline, VkExtent2D extent);
void renderer_draw_mesh(Renderer *renderer, Pipeline *pipeline,
                           Mesh *mesh, VkExtent2D extent);
void renderer_update_ubo(Renderer *renderer, void *data);
i32 renderer_frame_end(Renderer *renderer, Device *device,
                          Swapchain *swapchain);

#endif
