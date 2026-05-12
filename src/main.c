#include "app.h"
#include "core.h"
#include "window.h"
#include <stdio.h>

#define VS_FAIL(app, window, msg)                                              \
  do {                                                                         \
    fprintf(stderr, "%s\n", msg);                                              \
    vs_window_destroy(window);                                                 \
    vs_app_destroy(app);                                                       \
    vs_terminate();                                                            \
    return 1;                                                                  \
  } while (0)

int main(void) {
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

  printf("Using GPU: %s\n", vs_app_gpu_name(app));

  if (!vs_renderer_create_pipeline(
          app->renderer, app->device, app->swapchain->format,
          SHADER_DIR "/triangle.vert.spv", SHADER_DIR "/triangle.frag.spv"))
    VS_FAIL(app, window, "failed to create pipeline");

  while (!vs_window_should_close(window)) {
    glfwPollEvents();
    vs_app_set_clear_color(app, VS_COLOR_BLACK);
    vs_app_draw(app, window);
  }

  vs_window_destroy(window);
  vs_app_destroy(app);
  vs_terminate();
  return 0;
}
