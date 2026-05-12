#ifndef DEVICE_H
#define DEVICE_H
#include "utils/globals.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

typedef struct {
  VkPhysicalDevice physical;
  VkDevice device;
  VkQueue qQueue;
  VkQueue pQueue;
  u32 graphicsFamily;
  u32 presentFamily;
} VsDevice;

VsDevice *vs_device_create(VkInstance instance, VkSurfaceKHR surface);
void vs_device_destroy(VsDevice *device);

// helper functions
i32 find_queue_family(VkPhysicalDevice physical, VkSurfaceKHR surface,
                      i32 *graphicsFamily, i32 *presentFamily);

#endif // !DEVICE_H
