#include "swapchain.h"
#include <stdio.h>
#include <stdlib.h>

VsSwapchain *vs_swapchain_create(VsDevice *device, VkSurfaceKHR surface,
                                 uint32_t width, uint32_t height) {
  VsSwapchain *swapchain = malloc(sizeof(VsSwapchain));
  if (!swapchain) {
    fprintf(stderr, "failed to allocate swapchain\n");
    return NULL;
  }

  swapchain->device = device->device;

  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical, surface, &caps);

  if (caps.currentExtent.width != UINT32_MAX) {
    swapchain->extent = caps.currentExtent;
  } else {
    swapchain->extent.width =
        width < caps.minImageExtent.width   ? caps.minImageExtent.width
        : width > caps.maxImageExtent.width ? caps.maxImageExtent.width
                                            : width;
    swapchain->extent.height =
        height < caps.minImageExtent.height   ? caps.minImageExtent.height
        : height > caps.maxImageExtent.height ? caps.maxImageExtent.height
                                              : height;
  }

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &formatCount,
                                       NULL);
  VkSurfaceFormatKHR *formats =
      malloc(formatCount * sizeof(VkSurfaceFormatKHR));
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &formatCount,
                                       formats);

  VkSurfaceFormatKHR surfaceFormat = formats[0];
  for (uint32_t i = 0; i < formatCount; i++) {
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      surfaceFormat = formats[i];
      break;
    }
  }
  free(formats);
  swapchain->format = surfaceFormat.format;

  uint32_t modeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface,
                                            &modeCount, NULL);
  VkPresentModeKHR *modes = malloc(modeCount * sizeof(VkPresentModeKHR));
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface,
                                            &modeCount, modes);

  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (uint32_t i = 0; i < modeCount; i++) {
    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
      break;
    }
  }
  free(modes);

  uint32_t imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
    imageCount = caps.maxImageCount;

  VkSwapchainCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = imageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = swapchain->extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = caps.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
  };

  if (vkCreateSwapchainKHR(device->device, &createInfo, NULL,
                           &swapchain->handle) != VK_SUCCESS) {
    fprintf(stderr, "failed to create swapchain\n");
    free(swapchain);
    return NULL;
  }

  vkGetSwapchainImagesKHR(device->device, swapchain->handle, &swapchain->count,
                          NULL);
  swapchain->images = malloc(swapchain->count * sizeof(VkImage));
  vkGetSwapchainImagesKHR(device->device, swapchain->handle, &swapchain->count,
                          swapchain->images);

  swapchain->views = malloc(swapchain->count * sizeof(VkImageView));
  for (uint32_t i = 0; i < swapchain->count; i++) {
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapchain->images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain->format,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    if (vkCreateImageView(device->device, &viewInfo, NULL,
                          &swapchain->views[i]) != VK_SUCCESS) {
      fprintf(stderr, "failed to create image view\n");
      for (uint32_t j = 0; j < i; j++)
        vkDestroyImageView(device->device, swapchain->views[j], NULL);
      vkDestroySwapchainKHR(device->device, swapchain->handle, NULL);
      free(swapchain->images);
      free(swapchain->views);
      free(swapchain);
      return NULL;
    }
  }

  return swapchain;
}

void vs_swapchain_destroy(VsSwapchain *swapchain) {
  if (!swapchain)
    return;
  for (uint32_t i = 0; i < swapchain->count; i++)
    vkDestroyImageView(swapchain->device, swapchain->views[i], NULL);
  vkDestroySwapchainKHR(swapchain->device, swapchain->handle, NULL);
  free(swapchain->images);
  free(swapchain->views);
  free(swapchain);
}
