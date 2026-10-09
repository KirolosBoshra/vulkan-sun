#include "app/app.h"
#include "camera.h"
#include "core/core.h"
#include "core/logger.h"
#include "core/window.h"
#include "render/types/mesh.h"
#include "utils/globals.h"

#define FAIL(app, window, msg)                                                 \
  do {                                                                         \
    log_error("%s", msg);                                                      \
    window_destroy(window);                                                    \
    app_destroy(app);                                                          \
    terminate();                                                               \
    return 1;                                                                  \
  } while (0)

typedef struct {
  f32 time;
} BackgroundPush;

const i32 WIDTH = 600;
const i32 HEIGHT = 400;

i32 main(void) {
  log_info("starting vulkan_sun");
  init();

  App *app = NULL;
  Window *window = NULL;

  app = app_create("Vulkan Sun", VK_MAKE_API_VERSION(0, 0, 1, 0));
  if (!app)
    FAIL(app, window, "failed to create app");

  window = window_create(app->instance, WIDTH, HEIGHT, "Vulkan Sun");
  if (!window)
    FAIL(app, window, "failed to create window");

  app_setup(app, window->surface, WIDTH, HEIGHT);

  if (!app->device || !app->swapchain || !app->renderer)
    FAIL(app, window, "failed to setup app");

  i32 cubeIdx =
      add_pipeline(app, SHADER_DIR "/cube.vert.spv",
                   SHADER_DIR "/cube.frag.spv", (u32)sizeof(ModelPush));
  log_info("cube pipeline index: %d", cubeIdx);
  if (cubeIdx < 0)
    FAIL(app, window, "failed to create cube pipeline");

  i32 backgroundIdx = add_pipeline_ex(
      app, SHADER_DIR "/background.vert.spv", SHADER_DIR "/background.frag.spv",
      (u32)sizeof(BackgroundPush), NULL, 0, false);
  log_info("background pipeline index: %d", backgroundIdx);
  if (backgroundIdx < 0)
    FAIL(app, window, "failed to create background pipeline");

  VkDescriptorSetLayout texLayouts[] = {app->renderer->imageLayout};
  i32 texturedIdx = add_pipeline_ex(
      app, SHADER_DIR "/cube.vert.spv", SHADER_DIR "/textured.frag.spv",
      (u32)sizeof(ModelPush), texLayouts, 1, true);
  log_info("textured pipeline index: %d", texturedIdx);
  if (texturedIdx < 0)
    FAIL(app, window, "failed to create textured pipeline");

  Texture logoTex = load_texture(app, ASSETS_DIR "/logo.png");
  if (logoTex.image == INVALID_HANDLE)
    FAIL(app, window, "failed to load logo texture");

  VkDescriptorSet logoSet = texture_bind(app, &logoTex);
  if (logoSet == VK_NULL_HANDLE)
    FAIL(app, window, "failed to bind logo texture");

  Vertex cubeVerts[] = {CUBE_VERTICES};
  u32 cubeIndices[] = CUBE_INDICES;
  MeshHandle cubeHandle =
      upload_mesh(app, cubeVerts, sizeof(cubeVerts) / sizeof(Vertex),
                  cubeIndices, sizeof(cubeIndices) / sizeof(u32));
  if (cubeHandle == INVALID_HANDLE)
    FAIL(app, window, "failed to upload cube mesh");

  Model cubeModel = model_create(cubeHandle);
  cubeModel.color = COLOR_WHITE;
  Model smallModel = model_create(cubeHandle);
  smallModel.color = COLOR_RED;
  smallModel.transform = mat4_translate(
      mat4_scale(MAT4_IDENTITY, (Vec3){0.5f, 0.5f, 0.5f}), (Vec3){0, 2, 0});

  Camera cam = create_camera(WIDTH, HEIGHT);
  cam.position = (Vec3){0, 5, 5};
  log_info("entering main loop");

  while (!window_should_close(window)) {
    glfwPollEvents();

    f32 t = (f32)glfwGetTime();

    i32 result = frame_begin(app, COLOR_BLACK);
    if (result == 1) {
      app_resize(app, window);
      if (window->resized)
        window->resized = 0;
      else
        glfwWaitEvents();
      continue;
    }
    if (result < 0) {
      log_error("frame begin failed");
      break;
    }

    BackgroundPush bgt = {.time = t};
    push_constants(app, backgroundIdx, &bgt);
    draw(app, backgroundIdx);

    Mat4 vp = camera_vp(&cam);
    update_ubo(app, &vp);

    cubeModel.transform = mat4_rotate_y(MAT4_IDENTITY, t * 0.5f);
    draw_model_textured(app, texturedIdx, &cubeModel, logoSet);

    draw_model(app, cubeIdx, &smallModel);

    result = frame_end(app);
    if (result == 1) {
      app_resize(app, window);
      continue;
    }
    if (result < 0) {
      log_error("frame end failed");
      break;
    }
  }

  log_info("shutting down");
  window_destroy(window);
  app_destroy(app);
  terminate();
  return 0;
}
