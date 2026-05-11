#include "swapchain.h"
#include <stdio.h>
#include <stdlib.h>

static VkSurfaceFormatKHR pick_format(VkSurfaceFormatKHR *formats,
                                      uint32_t count) {
  for (uint32_t i = 0; i < count; i++)
    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return formats[i];
  return formats[0];
}

static VkPresentModeKHR pick_present_mode(VkPresentModeKHR *modes,
                                          uint32_t count) {
  for (uint32_t i = 0; i < count; i++)
    if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
      return modes[i];
  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D pick_extent(const VkSurfaceCapabilitiesKHR *caps,
                              uint32_t width, uint32_t height) {
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

// create image views for all swapchain images; if any fail, clean up and return
// -1
static int create_views(VsSwapchain *sc) {
  for (uint32_t i = 0; i < sc->count; i++) {
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
      fprintf(stderr, "failed to create image view %u\n", i);
      for (uint32_t j = 0; j < i; j++)
        vkDestroyImageView(sc->device, sc->views[j], NULL);
      return -1;
    }
  }
  return 0;
}

// Build a swapchain with the given parameters,
// optionally reusing resources from an old swapchain. Returns 0 on success, -1
// on failure.
static int swapchain_build(VsSwapchain *sc, VsDevice *device, uint32_t width,
                           uint32_t height, VkSwapchainKHR old) {
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical, sc->surface,
                                            &caps);

  uint32_t fmtCount = 0;
  VkSurfaceFormatKHR *fmts = NULL;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, sc->surface, &fmtCount,
                                       NULL);
  fmts = malloc(fmtCount * sizeof *fmts);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, sc->surface, &fmtCount,
                                       fmts);

  uint32_t modeCount = 0;
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

  uint32_t imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
    imageCount = caps.maxImageCount;

  uint32_t queueIndices[] = {device->graphicsFamily, device->presentFamily};

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

  // if two images are used across multiple queue families, we either have to
  // explicitly transfer ownership between the queues or we can create the
  // swapchain with concurrent sharing mode. We choose the latter in this
  // example for simplicity.
  if (device->graphicsFamily != device->presentFamily) {
    ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    ci.queueFamilyIndexCount = 2;
    ci.pQueueFamilyIndices = queueIndices;
  } else {
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  if (vkCreateSwapchainKHR(device->device, &ci, NULL, &sc->handle) !=
      VK_SUCCESS) {
    fprintf(stderr, "failed to create swapchain\n");
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
                                 uint32_t width, uint32_t height) {
  VsSwapchain *sc = malloc(sizeof *sc);
  if (!sc) {
    fprintf(stderr, "failed to allocate swapchain\n");
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
  return sc;
}

void vs_swapchain_recreate(VsSwapchain *sc, VsDevice *device, uint32_t width,
                           uint32_t height) {
  if (!sc)
    return;

  // destroy old image views
  for (uint32_t i = 0; i < sc->count; i++)
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
  if (!sc)
    return;
  for (uint32_t i = 0; i < sc->count; i++)
    vkDestroyImageView(sc->device, sc->views[i], NULL);
  vkDestroySwapchainKHR(sc->device, sc->handle, NULL);
  free(sc->images);
  free(sc->views);
  free(sc);
}
