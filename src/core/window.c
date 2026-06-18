#include "window.h"
#include "logger.h"
#include "utils/globals.h"
#include <stdlib.h>

static void framebuffer_resize_callback(GLFWwindow *handle, i32 w, i32 h) {
  (void)w;
  (void)h;
  Window *window = glfwGetWindowUserPointer(handle);
  if (window)
    window->resized = 1;
}

void glfw_init(void) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void glfw_terminate(void) { glfwTerminate(); }

Window *window_create(VkInstance instance, u32 width, u32 height,
                           const char *title) {
  log_trace("creating window: %ux%u", width, height);

  Window *window = malloc(sizeof(Window));
  if (!window) {
    log_error("failed to allocate window");
    return NULL;
  }

  window->instance = instance;

  window->handle = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window->handle) {
    log_error("failed to create glfw window");
    free(window);
    return NULL;
  }

  glfwSetWindowUserPointer(window->handle, window);
  glfwSetFramebufferSizeCallback(window->handle, framebuffer_resize_callback);

  if (glfwCreateWindowSurface(instance, window->handle, NULL,
                              &window->surface) != VK_SUCCESS) {
    log_error("failed to create window surface");
    glfwDestroyWindow(window->handle);
    free(window);
    return NULL;
  }

  log_trace("window created: %ux%u", width, height);
  return window;
}

void window_destroy(Window *window) {
  log_trace("destroying window");
  if (!window)
    return;
  vkDestroySurfaceKHR(window->instance, window->surface, NULL);
  glfwDestroyWindow(window->handle);
  free(window);
}

void window_get_size(Window *window, u32 *width, u32 *height) {
  i32 w = 0, h = 0;
  if (window)
    glfwGetFramebufferSize(window->handle, &w, &h);
  if (width)
    *width = (u32)w;
  if (height)
    *height = (u32)h;
}

bool window_should_close(Window *window) {
  return !window || glfwWindowShouldClose(window->handle);
}
