#ifndef APP_H
#define APP_H
#include "utils/globals.h"

#include "render/device.h"
#include "render/types/mesh.h"
#include "render/pipeline.h"
#include "render/renderer.h"
#include "render/resource.h"
#include "render/types/model.h"
#include "render/swapchain.h"
#include "core/window.h"
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
} App;

App *app_create(const char *name, u32 version);
void app_setup(App *app, VkSurfaceKHR surface, u32 width, u32 height);
void app_destroy(App *app);
const char *app_gpu_name(App *app);

i32 add_pipeline(App *app, const char *vertPath, const char *fragPath,
                    u32 pushSize);
i32 add_pipeline_ex(App *app, const char *vertPath, const char *fragPath,
                       u32 pushSize, VkDescriptorSetLayout *setLayouts,
                       u32 setLayoutCount, bool depthEnable);
void clear(App *app, Color color);
i32 frame_begin(App *app, Color clearColor);
void push_constants(App *app, i32 pipelineIndex, void *data);
void draw(App *app, i32 pipelineIndex);
void draw_mesh(App *app, i32 pipelineIndex, Mesh *mesh);
void draw_model(App *app, i32 pipelineIndex, Model *model);
i32 frame_end(App *app);
void app_resize(App *app, Window *window);
void update_ubo(App *app, void *data);

#endif
