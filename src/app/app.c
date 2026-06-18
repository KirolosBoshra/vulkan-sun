#include "app.h"
#include "core/logger.h"
#include "render/resource.h"
#include "render/types/model.h"
#include "utils/globals.h"
#include <stdlib.h>
#include <string.h>

App *app_create(const char *name, u32 version) {
  log_trace("creating Vulkan instance");

  App *app = malloc(sizeof(App));
  if (!app) {
    log_error("failed to allocate app");
    return NULL;
  }

  app->device = NULL;
  app->swapchain = NULL;
  app->renderer = NULL;
  app->resources = NULL;
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
    log_error("failed to create vulkan instance");
    free(app);
    return NULL;
  }

  return app;
}

void app_setup(App *app, VkSurfaceKHR surface, u32 width, u32 height) {
  log_trace("setting up device, swapchain, renderer");

  if (!app)
    return;

  app->device = device_create(app->instance, surface);
  if (!app->device) {
    log_error("failed to create device");
    return;
  }

  app->swapchain = swapchain_create(app->device, surface, width, height);
  if (!app->swapchain) {
    log_error("failed to create swapchain");
    device_destroy(app->device);
    app->device = NULL;
    return;
  }

  app->renderer = renderer_create(app->device, sizeof(Mat4));
  if (!app->renderer) {
    log_error("failed to create renderer");
    swapchain_destroy(app->swapchain);
    app->swapchain = NULL;
    device_destroy(app->device);
    app->device = NULL;
    return;
  }
  app->sceneLayout = app->renderer->sceneLayout;

  app->resources = create_resource_manager();
  if (!app->resources) {
    log_error("failed to create resource manager");
    renderer_destroy(app->renderer, app->device);
    app->renderer = NULL;
    swapchain_destroy(app->swapchain);
    app->swapchain = NULL;
    device_destroy(app->device);
    app->device = NULL;
    return;
  }
}

void app_destroy(App *app) {
  log_trace("destroying app");

  if (!app)
    return;

  if (app->device)
    vkDeviceWaitIdle(app->device->device);

  for (u32 i = 0; i < app->pipelineCount; i++)
    pipeline_destroy(app->pipelines[i], app->device->device);

  if (app->renderer)
    renderer_destroy(app->renderer, app->device);
  if (app->swapchain)
    swapchain_destroy(app->swapchain);
  if (app->resources)
    resource_manager_destroy(app->resources, app->device->device);
  if (app->device)
    device_destroy(app->device);

  vkDestroyInstance(app->instance, NULL);
  free(app);
}

const char *app_gpu_name(App *app) {
  if (!app || !app->device)
    return "unknown";

  static char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(app->device->physical, &props);
  memcpy(name, props.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
  return name;
}

i32 add_pipeline(App *app, const char *vertPath, const char *fragPath,
                    u32 pushSize) {
  return add_pipeline_ex(app, vertPath, fragPath, pushSize, NULL, 0, true);
}

i32 add_pipeline_ex(App *app, const char *vertPath, const char *fragPath,
                       u32 pushSize, VkDescriptorSetLayout *setLayouts,
                       u32 setLayoutCount, bool depthEnable) {
  if (!app || app->pipelineCount >= MAX_PIPELINES)
    return -1;

  Shader vert = shader_create(app->device->device,
                                   VK_SHADER_STAGE_VERTEX_BIT, vertPath);
  Shader frag = shader_create(app->device->device,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, fragPath);

  VkDescriptorSetLayout layouts[MAX_PIPELINES];
  layouts[0] = app->sceneLayout;
  for (u32 i = 0; i < setLayoutCount; i++)
    layouts[1 + i] = setLayouts[i];

  Pipeline *p = pipeline_create(
      app->device->device, app->swapchain->format, vert, frag, pushSize,
      layouts, 1 + setLayoutCount, depthEnable);

  shader_destroy(app->device->device, &vert);
  shader_destroy(app->device->device, &frag);

  if (!p)
    return -1;

  i32 idx = (i32)app->pipelineCount;
  app->pipelines[idx] = p;
  app->pipelineCount++;
  return idx;
}

void clear(App *app, Color color) {
  if (!app || !app->renderer)
    return;
  renderer_set_clear_color(app->renderer, color);
}

i32 frame_begin(App *app, Color clearColor) {
  if (!app || !app->renderer || !app->device || !app->swapchain)
    return -1;
  VkClearValue cv = color_clear_value(clearColor);
  return renderer_frame_begin(app->renderer, app->device, app->swapchain,
                                 cv);
}

void push_constants(App *app, i32 pipelineIndex, void *data) {
  if (!app || !app->renderer || pipelineIndex < 0 ||
      pipelineIndex >= (i32)app->pipelineCount)
    return;
  renderer_push_constants(app->renderer, app->pipelines[pipelineIndex], data);
}

void draw(App *app, i32 pipelineIndex) {
  if (!app || !app->renderer || pipelineIndex < 0 ||
      pipelineIndex >= (i32)app->pipelineCount)
    return;
  renderer_draw(app->renderer, app->pipelines[pipelineIndex],
                   app->swapchain->extent);
}

void draw_mesh(App *app, i32 pipelineIndex, Mesh *mesh) {
  if (!app || !app->renderer || !mesh || pipelineIndex < 0 ||
      pipelineIndex >= (i32)app->pipelineCount)
    return;
  renderer_draw_mesh(app->renderer, app->pipelines[pipelineIndex], mesh,
                        app->swapchain->extent);
}

void draw_model(App *app, i32 pipelineIndex, Model *model) {
  if (!app || !model || model->mesh == INVALID_HANDLE)
    return;

  Mesh *mesh = resource_get_mesh(app->resources, model->mesh);
  if (!mesh)
    return;

  Pipeline *p = app->pipelines[pipelineIndex];
  ModelPush mp = {.model = model->transform, .color = model->color};
  renderer_push_constants(app->renderer, p, &mp);
  renderer_draw_mesh(app->renderer, p, mesh, app->swapchain->extent);
}

i32 frame_end(App *app) {
  if (!app || !app->renderer || !app->device || !app->swapchain)
    return -1;
  return renderer_frame_end(app->renderer, app->device, app->swapchain);
}

void app_resize(App *app, Window *window) {
  if (!app || !window)
    return;
  u32 w = 0, h = 0;
  window_get_size(window, &w, &h);
  if (w && h) {
    vkDeviceWaitIdle(app->device->device);
    swapchain_recreate(app->swapchain, app->device, w, h);
  }
}

void update_ubo(App *app, void *data) {
  if (!app || !app->renderer)
    return;
  renderer_update_ubo(app->renderer, data);
}
