#include "app.h"
#include "logger.h"
#include "utils/globals.h"
#include <stdlib.h>
#include <string.h>

VsApp *vs_app_create(const char *name, u32 version) {
  vs_log_trace("creating Vulkan instance");

  VsApp *app = malloc(sizeof(VsApp));
  if (!app) {
    vs_log_error("failed to allocate app");
    return NULL;
  }

  app->device = NULL;
  app->swapchain = NULL;
  app->renderer = NULL;
  app->pipelineCount = 0;

  u32 extCount = 0;
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

void vs_app_setup(VsApp *app, VkSurfaceKHR surface, u32 width, u32 height) {
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

  for (u32 i = 0; i < app->pipelineCount; i++)
    vs_pipeline_destroy(app->pipelines[i], app->device->device);

  if (app->renderer)
    vs_renderer_destroy(app->renderer, app->device);
  if (app->swapchain)
    vs_swapchain_destroy(app->swapchain);
  if (app->device)
    vs_device_destroy(app->device);

  vkDestroyInstance(app->instance, NULL);
  free(app);
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

i32 vs_add_pipeline(VsApp *app, const char *vertPath, const char *fragPath,
                    u32 pushSize) {
  if (!app || app->pipelineCount >= VS_MAX_PIPELINES)
    return -1;

  VsShader vert = vs_shader_create(app->device->device,
                                   VK_SHADER_STAGE_VERTEX_BIT, vertPath);
  VsShader frag = vs_shader_create(app->device->device,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, fragPath);

  VsPipeline *p = vs_pipeline_create(
      app->device->device, app->swapchain->format, vert, frag, pushSize);

  vs_shader_destroy(app->device->device, &vert);
  vs_shader_destroy(app->device->device, &frag);

  if (!p)
    return -1;

  i32 idx = (i32)app->pipelineCount;
  app->pipelines[idx] = p;
  app->pipelineCount++;
  return idx;
}

void vs_clear(VsApp *app, VsColor color) {
  if (!app || !app->renderer)
    return;
  vs_renderer_set_clear_color(app->renderer, color);
}

i32 vs_frame_begin(VsApp *app) {
  if (!app || !app->renderer || !app->device || !app->swapchain)
    return -1;
  return vs_renderer_frame_begin(app->renderer, app->device, app->swapchain);
}

void vs_draw(VsApp *app, i32 pipelineIndex) {
  if (!app || !app->renderer || pipelineIndex < 0 ||
      pipelineIndex >= (i32)app->pipelineCount)
    return;
  vs_renderer_draw(app->renderer, app->pipelines[pipelineIndex],
                   app->swapchain->extent);
}

i32 vs_frame_end(VsApp *app) {
  if (!app || !app->renderer || !app->device || !app->swapchain)
    return -1;
  return vs_renderer_frame_end(app->renderer, app->device, app->swapchain);
}

void vs_app_resize(VsApp *app, VsWindow *window) {
  if (!app || !window)
    return;
  u32 w = 0, h = 0;
  vs_window_get_size(window, &w, &h);
  if (w && h) {
    vkDeviceWaitIdle(app->device->device);
    vs_swapchain_recreate(app->swapchain, app->device, w, h);
  }
}

void vs_push_constants(VsApp *app, void *ptr, u32 size) {
  if (!app || !app->renderer || size > sizeof(app->renderer->pushData))
    return;
  memcpy(app->renderer->pushData, ptr, size);
}
