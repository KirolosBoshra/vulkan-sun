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
  app->samplerNearest = VK_NULL_HANDLE;
  app->samplerLinear = VK_NULL_HANDLE;
  app->samplerNearestClamp = VK_NULL_HANDLE;
  app->samplerLinearClamp = VK_NULL_HANDLE;
  app->defaultChecker = (Texture){.image = INVALID_HANDLE};

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

  app_init_samplers(app);
  app_init_default_texture(app);
}

void app_init_default_texture(App *app) {
  if (!app || !app->resources || !app->device)
    return;

  // fallback texture for failed loads (16x16 pink/black checker)
  u8 magenta[] = {255, 0, 255, 255};
  u8 black[] = {0, 0, 0, 255};
  u8 pixels[16 * 16 * 4];
  for (i32 i = 0; i < 16 * 16; i++) {
    i32 x = i % 16, y = i / 16;
    u8 *src = ((x % 2) ^ (y % 2)) ? magenta : black;
    memcpy(&pixels[i * 4], src, 4);
  }

  ImageHandle handle = upload_image(app, pixels, (VkExtent3D){16, 16, 1},
                                    VK_FORMAT_R8G8B8A8_SRGB, 1);
  app->defaultChecker = create_texture(app, handle);
  app->defaultChecker.filter = TEX_FILTER_NEAREST;
}

void app_init_samplers(App *app) {
  if (!app || !app->device)
    return;

  VkSamplerCreateInfo sampl = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampl.magFilter = VK_FILTER_NEAREST;
  sampl.minFilter = VK_FILTER_NEAREST;
  sampl.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampl.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  vkCreateSampler(app->device->device, &sampl, NULL, &app->samplerNearest);

  sampl.magFilter = VK_FILTER_LINEAR;
  sampl.minFilter = VK_FILTER_LINEAR;
  vkCreateSampler(app->device->device, &sampl, NULL, &app->samplerLinear);

  sampl.magFilter = VK_FILTER_NEAREST;
  sampl.minFilter = VK_FILTER_NEAREST;
  sampl.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  sampl.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  vkCreateSampler(app->device->device, &sampl, NULL, &app->samplerNearestClamp);

  sampl.magFilter = VK_FILTER_LINEAR;
  sampl.minFilter = VK_FILTER_LINEAR;
  vkCreateSampler(app->device->device, &sampl, NULL, &app->samplerLinearClamp);
}

void app_destroy(App *app) {
  log_trace("destroying app");

  if (!app)
    return;

  if (app->device)
    vkDeviceWaitIdle(app->device->device);

  if (app->device) {
    vkDestroySampler(app->device->device, app->samplerNearest, NULL);
    vkDestroySampler(app->device->device, app->samplerLinear, NULL);
    vkDestroySampler(app->device->device, app->samplerNearestClamp, NULL);
    vkDestroySampler(app->device->device, app->samplerLinearClamp, NULL);
  }

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

  Shader vert =
      shader_create(app->device->device, VK_SHADER_STAGE_VERTEX_BIT, vertPath);
  Shader frag = shader_create(app->device->device, VK_SHADER_STAGE_FRAGMENT_BIT,
                              fragPath);

  VkDescriptorSetLayout layouts[MAX_PIPELINES];
  layouts[0] = app->sceneLayout;
  for (u32 i = 0; i < setLayoutCount; i++)
    layouts[1 + i] = setLayouts[i];

  Pipeline *p =
      pipeline_create(app->device->device, app->swapchain->format, vert, frag,
                      pushSize, layouts, 1 + setLayoutCount, depthEnable);

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
  return renderer_frame_begin(app->renderer, app->device, app->swapchain, cv);
}

void push_constants(App *app, i32 pipelineIndex, void *data) {
  if (!app || !app->renderer || pipelineIndex < 0 ||
      pipelineIndex >= (i32)app->pipelineCount)
    return;

  renderer_push_constants(app->renderer, app->pipelines[pipelineIndex], data);
}

MeshHandle upload_mesh(App *app, Vertex *verts, u32 vertCount, u32 *indices,
                       u32 indexCount) {
  if (!app || !app->resources || !app->device)
    return INVALID_HANDLE;

  return resource_upload_mesh(app->resources, app->device, verts, vertCount,
                              indices, indexCount);
}

ImageHandle upload_image(App *app, void *data, VkExtent3D size, VkFormat format,
                         u32 mipLevels) {
  if (!app || !app->resources || !app->device)
    return INVALID_HANDLE;

  return resource_upload_image(app->resources, app->device, data, size, format,
                               mipLevels);
}

ImageHandle create_image(App *app, VkExtent3D size, VkFormat format,
                         u32 mipLevels) {
  if (!app || !app->resources || !app->device)
    return INVALID_HANDLE;

  return resource_create_image(app->resources, app->device, size, format,
                               mipLevels);
}

// Texture used whenever a texture cannot be created or loaded.
static Texture texture_fallback(App *app) {
  if (app && app->defaultChecker.image != INVALID_HANDLE)
    return app->defaultChecker;
  return (Texture){.image = INVALID_HANDLE};
}

Texture create_texture(App *app, ImageHandle handle) {
  if (!app || !app->resources || handle == INVALID_HANDLE)
    return texture_fallback(app);

  Image *img = resource_get_image(app->resources, handle);
  if (!img)
    return texture_fallback(app);

  Texture tex = {0};
  tex.image = handle;
  tex.width = (i32)img->extent.width;
  tex.height = (i32)img->extent.height;
  tex.mipLevels = img->mipLevels;
  tex.filter = TEX_FILTER_LINEAR;
  tex.wrap = TEX_WRAP_REPEAT;
  return tex;
}

Texture load_texture(App *app, const char *path) {
  if (!app || !app->resources || !app->device || !path)
    return texture_fallback(app);

  Image img = image_load(app->device, path);
  if (!img.image) {
    log_warn("using the default texture, '%s' could not be loaded", path);
    return texture_fallback(app);
  }

  ImageHandle handle = store_image(app->resources, img);
  if (handle == INVALID_HANDLE) {
    image_destroy(&img, app->device->device);
    return texture_fallback(app);
  }

  return create_texture(app, handle);
}

static VkSampler sampler_for(App *app, TexFilter filter, TexWrap wrap) {
  if (wrap == TEX_WRAP_CLAMP)
    return filter == TEX_FILTER_NEAREST ? app->samplerNearestClamp
                                         : app->samplerLinearClamp;
  return filter == TEX_FILTER_NEAREST ? app->samplerNearest : app->samplerLinear;
}

VkDescriptorSet texture_bind(App *app, Texture *tex) {
  if (!app || !app->resources || !app->device || !app->renderer || !tex ||
      tex->image == INVALID_HANDLE)
    return VK_NULL_HANDLE;

  Image *img = resource_get_image(app->resources, tex->image);
  if (!img)
    return VK_NULL_HANDLE;

  VkSampler sampler = sampler_for(app, tex->filter, tex->wrap);

  VkDescriptorSet set =
      desc_alloc_allocate(&app->renderer->descAlloc, app->device->device,
                          app->renderer->imageLayout);
  if (set == VK_NULL_HANDLE)
    return VK_NULL_HANDLE;

  VkDescriptorBufferInfo bufInfos[1];
  VkDescriptorImageInfo imgInfos[1];
  VkWriteDescriptorSet writes[1];
  DescriptorWriter w;
  desc_writer_init(&w, bufInfos, imgInfos, writes, 1);
  desc_writer_write_image(&w, 0, img->view, sampler,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  desc_writer_update(&w, app->device->device, set);
  return set;
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

void draw_model_textured(App *app, i32 pipelineIndex, Model *model,
                         VkDescriptorSet texSet) {
  if (!app || !model || model->mesh == INVALID_HANDLE)
    return;

  Mesh *mesh = resource_get_mesh(app->resources, model->mesh);
  if (!mesh)
    return;

  Pipeline *p = app->pipelines[pipelineIndex];
  ModelPush mp = {.model = model->transform, .color = model->color};
  renderer_push_constants(app->renderer, p, &mp);
  renderer_draw_mesh_textured(app->renderer, p, mesh, app->swapchain->extent,
                              texSet);
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
