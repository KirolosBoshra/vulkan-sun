#include "app.h"
#include "core.h"
#include "window.h"
#include <stdio.h>

int main(void) {
  vs_init();

  VsApp *app = vs_app_create("Vulkan Sun", VK_MAKE_API_VERSION(0, 0, 1, 0));
  if (!app) {
    vs_terminate();
    return 1;
  }

  VsWindow *window = vs_window_create(app->instance, 600, 400, "Vulkan Sun");
  if (!window) {
    vs_app_destroy(app);
    vs_terminate();
    return 1;
  }

  vs_app_setup(app, window->surface, 600, 400);

  if (!app->device || !app->swapchain || !app->renderer) {
    vs_window_destroy(window);
    vs_app_destroy(app);
    vs_terminate();
    return 1;
  }

  printf("Using GPU: %s\n", vs_app_gpu_name(app));

  while (!vs_window_should_close(window)) {
    glfwPollEvents();
    vs_app_set_clear_color(app, VS_COLOR_BLUE);
    vs_app_draw(app);
  }

  vs_window_destroy(window);
  vs_app_destroy(app);
  vs_terminate();
  return 0;
}
