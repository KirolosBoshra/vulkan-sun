#ifndef APP_H
#define APP_H
#include "render/types/texture.h"
#include "utils/globals.h"

#include "core/window.h"
#include "render/device.h"
#include "render/pipeline.h"
#include "render/renderer.h"
#include "render/resource.h"
#include "render/swapchain.h"
#include "render/types/mesh.h"
#include "render/types/model.h"
#include <vulkan/vulkan.h>

#define MAX_PIPELINES 16

typedef struct {
  VkInstance instance;
  Device *device;
  Swapchain *swapchain;
  Renderer *renderer;
  Pipeline *pipelines[MAX_PIPELINES];
  VkDescriptorSetLayout sceneLayout;
  u32 pipelineCount;
  ResourceManager *resources;
  Texture defaultChecker;
  VkSampler samplerNearest;
  VkSampler samplerLinear;
  VkSampler samplerNearestClamp;
  VkSampler samplerLinearClamp;
} App;

App *app_create(const char *name, u32 version);
void app_setup(App *app, VkSurfaceKHR surface, u32 width, u32 height);
void app_init_samplers(App *app);
void app_init_default_texture(App *app);
void app_destroy(App *app);
const char *app_gpu_name(App *app);

i32 add_pipeline(App *app, const char *vertPath, const char *fragPath,
                 u32 pushSize);
i32 add_pipeline_ex(App *app, const char *vertPath, const char *fragPath,
                    u32 pushSize, VkDescriptorSetLayout *setLayouts,
                    u32 setLayoutCount, bool depthEnable);
void clear(App *app, Color color);
i32 frame_begin(App *app, Color clearColor);
i32 frame_end(App *app);
void push_constants(App *app, i32 pipelineIndex, void *data);
MeshHandle upload_mesh(App *app, Vertex *verts, u32 vertCount, u32 *indices,
                       u32 indexCount);
ImageHandle create_image(App *app, VkExtent3D size, VkFormat format,
                         u32 mipLevels);
ImageHandle upload_image(App *app, void *data, VkExtent3D size, VkFormat format,
                         u32 mipLevels);
Texture create_texture(App *app, ImageHandle handle);
Texture load_texture(App *app, const char *path);
VkDescriptorSet texture_bind(App *app, Texture *tex);
void draw(App *app, i32 pipelineIndex);
void draw_mesh(App *app, i32 pipelineIndex, Mesh *mesh);
void draw_model(App *app, i32 pipelineIndex, Model *model);
void draw_model_textured(App *app, i32 pipelineIndex, Model *model,
                         VkDescriptorSet texSet);
void app_resize(App *app, Window *window);
void update_ubo(App *app, void *data);

#endif
