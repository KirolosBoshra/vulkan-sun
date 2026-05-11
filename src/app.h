#ifndef APP_H
#define APP_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

typedef struct {
  VkInstance instance;
} VsApp;

VsApp *vs_app_create(const char *name, uint32_t version);
void vs_app_destroy(VsApp *app);

#endif
