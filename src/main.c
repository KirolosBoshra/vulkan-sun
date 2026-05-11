#include "app.h"
#include "core.h"
#include "window.h"

int main(void) {
  vs_init();

  VsApp *app = vs_app_create("Vulken Sun", VK_MAKE_API_VERSION(0, 0, 1, 0));
  if (!app) {
    vs_terminate();
    return 1;
  }

  VsWindow *window = vs_window_create(app->instance, 600, 400, "Vulken Sun");

  if (!window) {
    vs_app_destroy(app);
    vs_terminate();
    return 1;
  }

  while (!vs_window_should_close(window))
    glfwPollEvents();

  vs_window_destroy(window);
  vs_app_destroy(app);
  vs_terminate();
  return 0;
}
