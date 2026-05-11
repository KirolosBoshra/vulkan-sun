#ifndef DEVICE_H
#define DEVICE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

typedef struct {
  VkPhysicalDevice physical;
  VkDevice device;
  VkQueue qQueue;
  VkQueue pQueue;
  uint32_t graphicsFamily;
  uint32_t presentFamily;
} VsDevice;

VsDevice *vs_device_create(VkInstance instance, VkSurfaceKHR surface);
void vs_device_destroy(VsDevice *device);

// helper functions
int find_queue_family(VkPhysicalDevice physical, VkSurfaceKHR surface,
                      int *graphicsFamily, int *presentFamily);

#endif // !DEVICE_H
