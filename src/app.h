#ifndef APP_H
#define APP_H

#include "device.h"
#include "pipline.h"
#include "renderer.h"
#include "swapchain.h"
#include "window.h"
#include <vulkan/vulkan.h>

#define VS_MAX_PIPELINES 16

typedef struct {
  VkInstance instance;
  VsDevice *device;
  VsSwapchain *swapchain;
  VsRenderer *renderer;
  VsPipeline *pipelines[VS_MAX_PIPELINES];
  uint32_t pipelineCount;
} VsApp;

VsApp *vs_app_create(const char *name, uint32_t version);
void vs_app_setup(VsApp *app, VkSurfaceKHR surface, uint32_t width,
                  uint32_t height);
void vs_app_destroy(VsApp *app);
const char *vs_app_gpu_name(VsApp *app);

int vs_add_pipeline(VsApp *app, const char *vertPath, const char *fragPath);
void vs_clear(VsApp *app, VsColor color);
int vs_frame_begin(VsApp *app);
void vs_draw(VsApp *app, int pipelineIndex);
int vs_frame_end(VsApp *app);
void vs_app_resize(VsApp *app, VsWindow *window);

#endif
