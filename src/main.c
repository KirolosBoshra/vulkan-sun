#include "app.h"
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

i32 main(void) {
  vs_log_info("starting vulkan_sun");
  vs_init();

  VsApp *app = NULL;
  VsWindow *window = NULL;

  app = vs_app_create("Vulkan Sun", VK_MAKE_API_VERSION(0, 0, 1, 0));
  if (!app)
    VS_FAIL(app, window, "failed to create app");

  window = vs_window_create(app->instance, 600, 400, "Vulkan Sun");
  if (!window)
    VS_FAIL(app, window, "failed to create window");

  vs_app_setup(app, window->surface, 600, 400);

  if (!app->device || !app->swapchain || !app->renderer)
    VS_FAIL(app, window, "failed to setup app");

  i32 triIdx = vs_add_pipeline(app, SHADER_DIR "/triangle.vert.spv",
                               SHADER_DIR "/triangle.frag.spv", 0);
  vs_log_info("triangle pipeline index: %d", triIdx);
  if (triIdx < 0)
    VS_FAIL(app, window, "failed to create triangle pipeline");

  i32 backgroundIdx = vs_add_pipeline(app, SHADER_DIR "/background.vert.spv",
                                      SHADER_DIR "/background.frag.spv",
                                      (u32)sizeof(BackgroundPush));
  vs_log_info("background pipeline index: %d", backgroundIdx);
  if (backgroundIdx < 0)
    VS_FAIL(app, window, "failed to create background pipeline");

  // tmp mesh, we should really have some kind of resource manager
  Vertex verts[] = {
      {{-0.5, -0.5, 0}, {0, 0, 1}, VS_COLOR_RED, 0, 0},
      {{0.5, -0.5, 0}, {0, 0, 1}, VS_COLOR_GREEN, 0, 0},
      {{0.5, 0.5, 0}, {0, 0, 1}, VS_COLOR_BLUE, 0, 0},
      {{-0.5, 0.5, 0}, {0, 0, 1}, VS_COLOR_SKYBLUE, 0, 0},
  };
  u32 indices[] = {0, 1, 2, 0, 2, 3};

  Mesh quad = vs_mesh_create(app->device, verts, 4, indices, 6);

  vs_log_info("entering main loop");

  while (!vs_window_should_close(window)) {
    glfwPollEvents();

    i32 result = vs_frame_begin(app);
    if (result == 1) {
      vs_app_resize(app, window);
      continue;
    }
    if (result < 0) {
      vs_log_error("frame begin failed");
      break;
    }

    vs_clear(app, VS_COLOR_BLACK);

    BackgroundPush t = {.time = glfwGetTime()};
    vs_push_constants(app, &t, sizeof(t));
    vs_draw(app, backgroundIdx);
    vs_draw_mesh(app, triIdx, &quad);

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
  vs_mesh_destroy(&quad, app->device->device);
  vs_window_destroy(window);
  vs_app_destroy(app);
  vs_terminate();
  return 0;
}
