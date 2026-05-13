#ifndef APP_H
#define APP_H
#include "utils/globals.h"

#include "device.h"
#include "mesh.h"
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
  u32 pipelineCount;
} VsApp;

VsApp *vs_app_create(const char *name, u32 version);
void vs_app_setup(VsApp *app, VkSurfaceKHR surface, u32 width, u32 height);
void vs_app_destroy(VsApp *app);
const char *vs_app_gpu_name(VsApp *app);

i32 vs_add_pipeline(VsApp *app, const char *vertPath, const char *fragPath,
                    u32 pushSize);
void vs_clear(VsApp *app, VsColor color);
i32 vs_frame_begin(VsApp *app);
void vs_draw(VsApp *app, i32 pipelineIndex);
void vs_draw_mesh(VsApp *app, i32 pipelineIndex, Mesh *mesh);
i32 vs_frame_end(VsApp *app);
void vs_app_resize(VsApp *app, VsWindow *window);
void vs_push_constants(VsApp *app, void *ptr, u32 size);

#endif
