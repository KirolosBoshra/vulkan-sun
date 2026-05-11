#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct {
  GLFWwindow *handle;
  VkSurfaceKHR surface;
  VkInstance instance;
  int resized;
} VsWindow;

VsWindow *vs_window_create(VkInstance instance, uint32_t width, uint32_t height,
                           const char *title);
void vs_window_destroy(VsWindow *window);
bool vs_window_should_close(VsWindow *window);
void vs_window_get_size(VsWindow *window, uint32_t *width, uint32_t *height);

#endif
