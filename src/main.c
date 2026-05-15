#include "app.h"
#include "camera.h"
#include "core.h"
#include "logger.h"
#include "utils/globals.h"
#include "utils/types.h"
#include "window.h"

#define VS_FAIL(app, window, msg)                                              \
  do {                                                                         \
    vs_log_error("%s", msg);                                                   \
    vs_window_destroy(window);                                                 \
    vs_app_destroy(app);                                                       \
    vs_terminate();                                                            \
    return 1;                                                                  \
  } while (0)

typedef struct {
  f32 time;
} BackgroundPush;

const i32 WIDTH = 600;
const i32 HEIGHT = 400;

i32 main(void) {
  vs_log_info("starting vulkan_sun");
  vs_init();

  VsApp *app = NULL;
  VsWindow *window = NULL;

  app = vs_app_create("Vulkan Sun", VK_MAKE_API_VERSION(0, 0, 1, 0));
  if (!app)
    VS_FAIL(app, window, "failed to create app");

  window = vs_window_create(app->instance, WIDTH, HEIGHT, "Vulkan Sun");
  if (!window)
    VS_FAIL(app, window, "failed to create window");

  vs_app_setup(app, window->surface, WIDTH, HEIGHT);

  if (!app->device || !app->swapchain || !app->renderer)
    VS_FAIL(app, window, "failed to setup app");

  i32 cubeIdx = vs_add_pipeline(app, SHADER_DIR "/cube.vert.spv",
                                SHADER_DIR "/cube.frag.spv", (u32)sizeof(Mat4));
  vs_log_info("cube pipeline index: %d", cubeIdx);
  if (cubeIdx < 0)
    VS_FAIL(app, window, "failed to create cube pipeline");

  i32 backgroundIdx = vs_add_pipeline(app, SHADER_DIR "/background.vert.spv",
                                      SHADER_DIR "/background.frag.spv",
                                      (u32)sizeof(BackgroundPush));
  vs_log_info("background pipeline index: %d", backgroundIdx);
  if (backgroundIdx < 0)
    VS_FAIL(app, window, "failed to create background pipeline");

  // tmp mesh, we should really have some kind of resource manager
  Vertex verts[] = {
      {{-1, -1, -1}, {0, 0, -1}, VS_COLOR_SKYBLUE, 0, 0},
      {{1, -1, -1}, {0, 0, -1}, VS_COLOR_SKYBLUE, 0, 0},
      {{1, 1, -1}, {0, 0, -1}, VS_COLOR_SKYBLUE, 0, 0},
      {{-1, 1, -1}, {0, 0, -1}, VS_COLOR_SKYBLUE, 0, 0},
      {{-1, -1, 1}, {0, 0, 1}, VS_COLOR_SKYBLUE, 0, 0},
      {{1, -1, 1}, {0, 0, 1}, VS_COLOR_SKYBLUE, 0, 0},
      {{1, 1, 1}, {0, 0, 1}, VS_COLOR_SKYBLUE, 0, 0},
      {{-1, 1, 1}, {0, 0, 1}, VS_COLOR_SKYBLUE, 0, 0},
  };
  u32 indices[] = {
      0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 0, 7, 4, 0, 3, 7,
      1, 5, 6, 1, 6, 2, 0, 5, 1, 0, 4, 5, 3, 6, 7, 3, 2, 6,
  };

  Mesh cube = vs_mesh_create(app->device, verts, 8, indices,
                             sizeof(indices) / sizeof(*indices));

  f32 s = 0.5f;
  Vertex smallVerts[] = {
      {{-s, -s, -s}, {0, 0, -1}, VS_COLOR_RED, 0, 0},
      {{s, -s, -s}, {0, 0, -1}, VS_COLOR_RED, 0, 0},
      {{s, s, -s}, {0, 0, -1}, VS_COLOR_RED, 0, 0},
      {{-s, s, -s}, {0, 0, -1}, VS_COLOR_RED, 0, 0},
      {{-s, -s, s}, {0, 0, 1}, VS_COLOR_RED, 0, 0},
      {{s, -s, s}, {0, 0, 1}, VS_COLOR_RED, 0, 0},
      {{s, s, s}, {0, 0, 1}, VS_COLOR_RED, 0, 0},
      {{-s, s, s}, {0, 0, 1}, VS_COLOR_RED, 0, 0},
  };
  Mesh smallCube = vs_mesh_create(app->device, smallVerts, 8, indices,
                                  sizeof(indices) / sizeof(*indices));

  Camera cam = create_camera(WIDTH, HEIGHT);
  cam.position = (Vec3){0, 5, 5};
  vs_log_info("entering main loop");

  while (!vs_window_should_close(window)) {
    glfwPollEvents();

    i32 result = vs_frame_begin(app, VS_COLOR_BLACK);
    if (result == 1) {
      vs_app_resize(app, window);
      continue;
    }
    if (result < 0) {
      vs_log_error("frame begin failed");
      break;
    }

    f32 t = (f32)glfwGetTime();
    BackgroundPush bgt = {.time = t};
    vs_push_constants(app, &bgt, sizeof(bgt));
    vs_draw(app, backgroundIdx);

    Mat4 vp = camera_vp(&cam);
    vs_update_ubo(app, &vp, sizeof(vp));

    Mat4 model = mat4_rotate_y(MAT4_IDENTITY, t * 0.5f);
    vs_push_constants(app, &model, sizeof(model));
    vs_draw_mesh(app, cubeIdx, &cube);

    Mat4 model2 = mat4_translation((Vec3){0, 2, 0});
    vs_push_constants(app, &model2, sizeof(model2));
    vs_draw_mesh(app, cubeIdx, &smallCube);

    result = vs_frame_end(app);
    if (result == 1) {
      vs_app_resize(app, window);
      continue;
    }
    if (result < 0) {
      vs_log_error("frame end failed");
      break;
    }
  }

  vs_log_info("shutting down");
  vs_mesh_destroy(&cube, app->device->device);
  vs_mesh_destroy(&smallCube, app->device->device);
  vs_window_destroy(window);
  vs_app_destroy(app);
  vs_terminate();
  return 0;
}
