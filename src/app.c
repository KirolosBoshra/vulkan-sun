#include "app.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

VsApp *vs_app_create(const char *name, uint32_t version) {
  vs_log_trace("creating Vulkan instance");

  VsApp *app = malloc(sizeof(VsApp));
  if (!app) {
    vs_log_error("failed to allocate app");
    return NULL;
  }

  app->device = NULL;
  app->swapchain = NULL;
  app->renderer = NULL;

  uint32_t extCount = 0;
  const char **extensions = glfwGetRequiredInstanceExtensions(&extCount);

  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = name,
      .applicationVersion = version,
      .pEngineName = "vulkan_sun",
      .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  VkInstanceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = extCount,
      .ppEnabledExtensionNames = extensions,
  };

  if (vkCreateInstance(&createInfo, NULL, &app->instance) != VK_SUCCESS) {
    vs_log_error("failed to create vulkan instance");
    free(app);
    return NULL;
  }

  return app;
}

void vs_app_setup(VsApp *app, VkSurfaceKHR surface, uint32_t width,
                  uint32_t height) {
  vs_log_trace("setting up device, swapchain, renderer");

  if (!app)
    return;

  app->device = vs_device_create(app->instance, surface);
  if (!app->device)
    return;

  app->swapchain = vs_swapchain_create(app->device, surface, width, height);
  if (!app->swapchain) {
    vs_device_destroy(app->device);
    app->device = NULL;
    return;
  }

  app->renderer = vs_renderer_create(app->device);
  if (!app->renderer) {
    vs_swapchain_destroy(app->swapchain);
    app->swapchain = NULL;
    vs_device_destroy(app->device);
    app->device = NULL;
  }
}

void vs_app_destroy(VsApp *app) {
  vs_log_trace("destroying app");

  if (!app)
    return;

  if (app->device)
    vkDeviceWaitIdle(app->device->device);

  if (app->renderer)
    vs_renderer_destroy(app->renderer, app->device);
  if (app->swapchain)
    vs_swapchain_destroy(app->swapchain);
  if (app->device)
    vs_device_destroy(app->device);

  vkDestroyInstance(app->instance, NULL);
  free(app);
}

void vs_app_set_clear_color(VsApp *app, VsColor color) {
  if (!app || !app->renderer)
    return;
  vs_renderer_set_clear_color(app->renderer, color);
}

void vs_app_draw(VsApp *app, VsWindow *window) {
  if (!app || !app->renderer || !app->device || !app->swapchain)
    return;

  int result = vs_renderer_draw(app->renderer, app->device, app->swapchain);

  if (result == 1 || window->resized) {
    window->resized = 0;

    uint32_t w = 0, h = 0;
    while (!w || !h) {
      vs_window_get_size(window, &w, &h);
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(app->device->device);
    vs_swapchain_recreate(app->swapchain, app->device, w, h);
  }
}

const char *vs_app_gpu_name(VsApp *app) {
  if (!app || !app->device)
    return "unknown";

  static char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(app->device->physical, &props);
  memcpy(name, props.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
  return name;
}
