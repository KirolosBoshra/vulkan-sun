#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "device.h"
#include "utils/globals.h"
#include "types/types.h"
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
  DepthImage depth;
} Swapchain;

Swapchain *swapchain_create(Device *device, VkSurfaceKHR surface,
                                 u32 width, u32 height);
void swapchain_recreate(Swapchain *swapchain, Device *device, u32 width,
                           u32 height);
void swapchain_destroy(Swapchain *swapchain);

#endif
