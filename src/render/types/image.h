#ifndef IMAGE_H
#define IMAGE_H

#include "render/device.h"
#include "utils/globals.h"
#include <vulkan/vulkan.h>

typedef struct {
  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  VkFormat format;
  VkExtent3D extent;
  u32 mipLevels;
} Image;

Image image_create(Device *device, VkExtent3D size, VkFormat format,
                   VkImageUsageFlags usage, VkImageAspectFlags aspect,
                   u32 mipLevels);

Image image_create_from_data(Device *device, void *data, VkExtent3D size,
                             VkFormat format, VkImageUsageFlags usage,
                             VkImageAspectFlags aspect, u32 mipLevels);

void transition_image_layout(VkCommandBuffer cmd, VkImage image,
                             VkFormat format, VkImageLayout from,
                             VkImageLayout to);

void image_destroy(Image *image, VkDevice device);

// Loads an image file from disk (dispatches on extension, currently .png),
// decodes to RGBA, and uploads to a sampled GPU image.
// Returns a zeroed Image on failure (check .image).
Image image_load(Device *device, const char *path);
#endif // !IMAGE_H
