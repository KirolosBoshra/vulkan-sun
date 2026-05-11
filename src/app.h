#ifndef APP_H
#define APP_H

#include "device.h"
#include "renderer.h"
#include "swapchain.h"
#include "window.h"
#include <vulkan/vulkan.h>

typedef struct {
  VkInstance instance;
  VsDevice *device;
  VsSwapchain *swapchain;
  VsRenderer *renderer;
} VsApp;

VsApp *vs_app_create(const char *name, uint32_t version);
void vs_app_setup(VsApp *app, VkSurfaceKHR surface, uint32_t width,
                  uint32_t height);
void vs_app_destroy(VsApp *app);
void vs_app_set_clear_color(VsApp *app, VsColor color);
void vs_app_draw(VsApp *app, VsWindow *window);
const char *vs_app_gpu_name(VsApp *app);

#endif
