#include "window.h"
#include <stdio.h>
#include <stdlib.h>

static void framebuffer_resize_callback(GLFWwindow *handle, int w, int h) {
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

VsWindow *vs_window_create(VkInstance instance, uint32_t width, uint32_t height,
                           const char *title) {
  VsWindow *window = malloc(sizeof(VsWindow));
  if (!window) {
    fprintf(stderr, "failed to allocate window\n");
    return NULL;
  }

  window->instance = instance;

  window->handle = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window->handle) {
    fprintf(stderr, "failed to create glfw window\n");
    free(window);
    return NULL;
  }

  if (glfwCreateWindowSurface(instance, window->handle, NULL,
                              &window->surface) != VK_SUCCESS) {
    fprintf(stderr, "failed to create window surface\n");
    glfwDestroyWindow(window->handle);
    free(window);
    return NULL;
  }

  // set user pointer and framebuffer resize callback
  glfwSetWindowUserPointer(window->handle, window);
  glfwSetFramebufferSizeCallback(window->handle, framebuffer_resize_callback);

  if (glfwCreateWindowSurface(instance, window->handle, NULL,
                              &window->surface) != VK_SUCCESS) {
    fprintf(stderr, "failed to create window surface\n");
    glfwDestroyWindow(window->handle);
    free(window);
    return NULL;
  }

  return window;
}

void vs_window_destroy(VsWindow *window) {
  if (!window)
    return;
  vkDestroySurfaceKHR(window->instance, window->surface, NULL);
  glfwDestroyWindow(window->handle);
  free(window);
}

void vs_window_get_size(VsWindow *window, uint32_t *width, uint32_t *height) {
  int w = 0, h = 0;
  if (window)
    glfwGetFramebufferSize(window->handle, &w, &h);
  if (width)
    *width = (uint32_t)w;
  if (height)
    *height = (uint32_t)h;
}

bool vs_window_should_close(VsWindow *window) {
  return !window || glfwWindowShouldClose(window->handle);
}
