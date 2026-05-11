#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "device.h"
#include <vulkan/vulkan.h>

typedef struct {
  VkSwapchainKHR handle;
  VkImage *images;
  VkImageView *views;
  VkFormat format;
  VkExtent2D extent;
  uint32_t count;
  VkDevice device;
} VsSwapchain;

VsSwapchain *vs_swapchain_create(VsDevice *device, VkSurfaceKHR surface,
                                uint32_t width, uint32_t height);
void vs_swapchain_destroy(VsSwapchain *swapchain);

#endif // SWAPCHAIN_H
#define SWAPCHAIN_H
