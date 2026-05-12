#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "device.h"
#include "utils/globals.h"
#include <vulkan/vulkan.h>

typedef struct {
  VkSwapchainKHR handle;
  VkImage *images;
  VkImageView *views;
  VkFormat format;
  VkExtent2D extent;
  u32 count;
  VkDevice device;
  VkSurfaceKHR surface;
} VsSwapchain;

VsSwapchain *vs_swapchain_create(VsDevice *device, VkSurfaceKHR surface,
                                 u32 width, u32 height);
void vs_swapchain_recreate(VsSwapchain *swapchain, VsDevice *device, u32 width,
                           u32 height);
void vs_swapchain_destroy(VsSwapchain *swapchain);

#endif
