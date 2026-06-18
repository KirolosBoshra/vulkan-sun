#ifndef WINDOW_H
#define WINDOW_H
#include "utils/globals.h"

#include <stdbool.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef struct {
  GLFWwindow *handle;
  VkSurfaceKHR surface;
  VkInstance instance;
  i32 resized;
} Window;

Window *window_create(VkInstance instance, u32 width, u32 height,
                           const char *title);
void window_destroy(Window *window);
bool window_should_close(Window *window);
void window_get_size(Window *window, u32 *width, u32 *height);

#endif
