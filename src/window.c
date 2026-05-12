#include "window.h"
#include "logger.h"
#include "utils/globals.h"
#include <stdlib.h>

static void framebuffer_resize_callback(GLFWwindow *handle, i32 w, i32 h) {
  (void)w;
  (void)h;
  VsWindow *window = glfwGetWindowUserPointer(handle);
  if (window)
    window->resized = 1;
}

void vs_glfw_init(void) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void vs_glfw_terminate(void) { glfwTerminate(); }

VsWindow *vs_window_create(VkInstance instance, u32 width, u32 height,
                           const char *title) {
  vs_log_trace("creating window: %ux%u", width, height);

  VsWindow *window = malloc(sizeof(VsWindow));
  if (!window) {
    vs_log_error("failed to allocate window");
    return NULL;
  }

  window->instance = instance;

  window->handle = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window->handle) {
    vs_log_error("failed to create glfw window");
    free(window);
    return NULL;
  }

  glfwSetWindowUserPointer(window->handle, window);
  glfwSetFramebufferSizeCallback(window->handle, framebuffer_resize_callback);

  if (glfwCreateWindowSurface(instance, window->handle, NULL,
                              &window->surface) != VK_SUCCESS) {
    vs_log_error("failed to create window surface");
    glfwDestroyWindow(window->handle);
    free(window);
    return NULL;
  }

  vs_log_trace("window created: %ux%u", width, height);
  return window;
}

void vs_window_destroy(VsWindow *window) {
  vs_log_trace("destroying window");
  if (!window)
    return;
  vkDestroySurfaceKHR(window->instance, window->surface, NULL);
  glfwDestroyWindow(window->handle);
  free(window);
}

void vs_window_get_size(VsWindow *window, u32 *width, u32 *height) {
  i32 w = 0, h = 0;
  if (window)
    glfwGetFramebufferSize(window->handle, &w, &h);
  if (width)
    *width = (u32)w;
  if (height)
    *height = (u32)h;
}

bool vs_window_should_close(VsWindow *window) {
  return !window || glfwWindowShouldClose(window->handle);
}
