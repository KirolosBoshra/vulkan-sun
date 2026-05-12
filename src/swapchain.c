#include "swapchain.h"
#include "logger.h"
#include "utils/globals.h"
#include <stdlib.h>

static VkSurfaceFormatKHR pick_format(VkSurfaceFormatKHR *formats, u32 count) {
  for (u32 i = 0; i < count; i++)
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return formats[i];
  return formats[0];
}

static VkPresentModeKHR pick_present_mode(VkPresentModeKHR *modes, u32 count) {
  for (u32 i = 0; i < count; i++)
    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
      return modes[i];
  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D pick_extent(const VkSurfaceCapabilitiesKHR *caps, u32 width,
                              u32 height) {
  if (caps->currentExtent.width != UINT32_MAX)
    return caps->currentExtent;

#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))
  return (VkExtent2D){
      .width =
          CLAMP(width, caps->minImageExtent.width, caps->maxImageExtent.width),
      .height = CLAMP(height, caps->minImageExtent.height,
                      caps->maxImageExtent.height),
  };
#undef CLAMP
}

static i32 create_views(VsSwapchain *sc) {
  for (u32 i = 0; i < sc->count; i++) {
    VkImageViewCreateInfo vi = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = sc->images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = sc->format,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    if (vkCreateImageView(sc->device, &vi, NULL, &sc->views[i]) != VK_SUCCESS) {
      vs_log_error("failed to create image view %u", i);
      for (u32 j = 0; j < i; j++)
        vkDestroyImageView(sc->device, sc->views[j], NULL);
      return -1;
    }
  }
  return 0;
}

static i32 swapchain_build(VsSwapchain *sc, VsDevice *device, u32 width,
                           u32 height, VkSwapchainKHR old) {
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical, sc->surface,
                                            &caps);

  u32 fmtCount = 0;
  VkSurfaceFormatKHR *fmts = NULL;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, sc->surface, &fmtCount,
                                       NULL);
  fmts = malloc(fmtCount * sizeof *fmts);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, sc->surface, &fmtCount,
                                       fmts);

  u32 modeCount = 0;
  VkPresentModeKHR *modes = NULL;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, sc->surface,
                                            &modeCount, NULL);
  modes = malloc(modeCount * sizeof *modes);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, sc->surface,
                                            &modeCount, modes);

  VkSurfaceFormatKHR fmt = pick_format(fmts, fmtCount);
  VkPresentModeKHR mode = pick_present_mode(modes, modeCount);
  VkExtent2D ext = pick_extent(&caps, width, height);
  free(fmts);
  free(modes);

  u32 imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
    imageCount = caps.maxImageCount;

  u32 queueIndices[] = {device->graphicsFamily, device->presentFamily};

  VkSwapchainCreateInfoKHR ci = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = sc->surface,
      .minImageCount = imageCount,
      .imageFormat = fmt.format,
      .imageColorSpace = fmt.colorSpace,
      .imageExtent = ext,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .preTransform = caps.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = mode,
      .clipped = VK_TRUE,
      .oldSwapchain = old,
  };

  if (device->graphicsFamily != device->presentFamily) {
    ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    ci.queueFamilyIndexCount = 2;
    ci.pQueueFamilyIndices = queueIndices;
  } else {
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  if (vkCreateSwapchainKHR(device->device, &ci, NULL, &sc->handle) !=
      VK_SUCCESS) {
    vs_log_error("failed to create swapchain");
    return -1;
  }

  sc->format = fmt.format;
  sc->extent = ext;
  sc->device = device->device;

  vkGetSwapchainImagesKHR(device->device, sc->handle, &sc->count, NULL);
  sc->images = malloc(sc->count * sizeof *sc->images);
  sc->views = malloc(sc->count * sizeof *sc->views);
  vkGetSwapchainImagesKHR(device->device, sc->handle, &sc->count, sc->images);

  if (create_views(sc) != 0) {
    vkDestroySwapchainKHR(device->device, sc->handle, NULL);
    free(sc->images);
    free(sc->views);
    sc->images = NULL;
    sc->views = NULL;
    return -1;
  }

  return 0;
}

VsSwapchain *vs_swapchain_create(VsDevice *device, VkSurfaceKHR surface,
                                 u32 width, u32 height) {
  vs_log_trace("creating swapchain: %ux%u", width, height);

  VsSwapchain *sc = malloc(sizeof *sc);
  if (!sc) {
    vs_log_error("failed to allocate swapchain");
    return NULL;
  }

  sc->surface = surface;
  sc->images = NULL;
  sc->views = NULL;
  sc->handle = VK_NULL_HANDLE;

  if (swapchain_build(sc, device, width, height, VK_NULL_HANDLE) != 0) {
    free(sc);
    return NULL;
  }

  vs_log_trace("swapchain created: %ux%u, %u images", sc->extent.width,
               sc->extent.height, sc->count);
  return sc;
}

void vs_swapchain_recreate(VsSwapchain *sc, VsDevice *device, u32 width,
                           u32 height) {
  vs_log_trace("recreating swapchain: %ux%u", width, height);

  if (!sc)
    return;

  for (u32 i = 0; i < sc->count; i++)
    vkDestroyImageView(sc->device, sc->views[i], NULL);
  free(sc->images);
  free(sc->views);
  sc->images = NULL;
  sc->views = NULL;

  VkSwapchainKHR old = sc->handle;
  sc->handle = VK_NULL_HANDLE;

  swapchain_build(sc, device, width, height, old);

  vkDestroySwapchainKHR(sc->device, old, NULL);
}

void vs_swapchain_destroy(VsSwapchain *sc) {
  vs_log_trace("destroying swapchain");
  if (!sc)
    return;
  for (u32 i = 0; i < sc->count; i++)
    vkDestroyImageView(sc->device, sc->views[i], NULL);
  vkDestroySwapchainKHR(sc->device, sc->handle, NULL);
  free(sc->images);
  free(sc->views);
  free(sc);
}
