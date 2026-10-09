#include "image.h"
#include "core/logger.h"
#include "types.h"
#include "utils/depng.h"
#include <string.h>

Image image_create(Device *device, VkExtent3D size, VkFormat format,
                   VkImageUsageFlags usage, VkImageAspectFlags aspect,
                   u32 mipLevels) {
  Image image = {0};

  VkImageCreateInfo imageInfo = {0};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent = size;
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(device->device, &imageInfo, NULL, &image.image) !=
      VK_SUCCESS) {
    log_error("Failed to create image!");
    return (Image){0};
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device->device, image.image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      find_memory_type(device->physical, memRequirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkAllocateMemory(device->device, &allocInfo, NULL, &image.memory) !=
      VK_SUCCESS) {
    log_error("Failed to allocate image memory!");
    vkDestroyImage(device->device, image.image, NULL);
    return (Image){0};
  }

  vkBindImageMemory(device->device, image.image, image.memory, 0);

  VkImageViewCreateInfo viewInfo = {0};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image.image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspect;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  image.format = format;
  image.extent = size;

  if (vkCreateImageView(device->device, &viewInfo, NULL, &image.view) !=
      VK_SUCCESS) {
    log_error("Failed to create image view!");
    vkFreeMemory(device->device, image.memory, NULL);
    vkDestroyImage(device->device, image.image, NULL);
    return (Image){0};
  }

  return image;
}

Image image_create_from_data(Device *device, void *data, VkExtent3D size,
                             VkFormat format, VkImageUsageFlags usage,
                             VkImageAspectFlags aspect, u32 mipLevels) {
  usize dataSize = size.width * size.height * size.depth * 4; // RGBA8

  // Staging buffer
  Buffer staging =
      create_buffer(dataSize, device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (!staging.buffer) {
    log_error("Failed to create staging buffer for image upload");
    return (Image){0};
  }

  void *mapped;
  vkMapMemory(device->device, staging.memory, 0, dataSize, 0, &mapped);
  memcpy(mapped, data, dataSize);
  vkUnmapMemory(device->device, staging.memory);

  // Target image (must be able to receive transfer)
  Image image =
      image_create(device, size, format,
                   usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, aspect, mipLevels);
  if (!image.image) {
    destroy_buffer(&staging, device->device);
    return (Image){0};
  }

  // Temp command pool + one-shot submit
  VkCommandPool pool;
  vkCreateCommandPool(device->device,
                      &(VkCommandPoolCreateInfo){
                          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                          .queueFamilyIndex = device->graphicsFamily,
                      },
                      NULL, &pool);

  VkCommandBuffer cmd;
  vkAllocateCommandBuffers(
      device->device,
      &(VkCommandBufferAllocateInfo){
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .commandPool = pool,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = 1,
      },
      &cmd);

  vkBeginCommandBuffer(cmd,
                       &(VkCommandBufferBeginInfo){
                           .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                           .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                       });

  // UNDEFINED → TRANSFER_DST
  transition_image_layout(cmd, image.image, format, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkBufferImageCopy region = {
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource =
          {
              .aspectMask = aspect,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .imageExtent = size,
  };
  vkCmdCopyBufferToImage(cmd, staging.buffer, image.image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  // TRANSFER_DST → SHADER_READ_ONLY
  transition_image_layout(cmd, image.image, format,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkEndCommandBuffer(cmd);

  vkQueueSubmit(device->qQueue, 1,
                &(VkSubmitInfo){
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &cmd,
                },
                VK_NULL_HANDLE);
  vkQueueWaitIdle(device->qQueue);
  vkDestroyCommandPool(device->device, pool, NULL);

  destroy_buffer(&staging, device->device);

  return image;
}

static VkImageAspectFlags format_to_aspect(VkFormat fmt) {
  if (fmt == VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D16_UNORM)
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  return VK_IMAGE_ASPECT_COLOR_BIT;
}

void transition_image_layout(VkCommandBuffer cmd, VkImage image,
                             VkFormat format, VkImageLayout from,
                             VkImageLayout to) {
  VkImageAspectFlags aspect = format_to_aspect(format);
  VkImageMemoryBarrier2 b = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .dstAccessMask =
          VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
      .oldLayout = from,
      .newLayout = to,
      .image = image,
      .subresourceRange = {.aspectMask = aspect,
                           .levelCount = VK_REMAINING_MIP_LEVELS,
                           .layerCount = VK_REMAINING_ARRAY_LAYERS},
  };
  VkDependencyInfo dep = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &b,
  };
  vkCmdPipelineBarrier2(cmd, &dep);
}

void image_destroy(Image *image, VkDevice device) {
  vkDestroyImageView(device, image->view, NULL);
  vkDestroyImage(device, image->image, NULL);
  vkFreeMemory(device, image->memory, NULL);

  log_trace("destroying image");
}

// ascii case-insensitive extension match ("png" matches ".png", ".PNG", ...)
static bool has_extension(const char *path, const char *ext) {
  const char *dot = strrchr(path, '.');
  if (!dot || !dot[1])
    return false;
  dot++;
  while (*dot && *ext) {
    char a = *dot >= 'A' && *dot <= 'Z' ? *dot + 32 : *dot;
    char b = *ext >= 'A' && *ext <= 'Z' ? *ext + 32 : *ext;
    if (a != b)
      return false;
    dot++;
    ext++;
  }
  return *dot == '\0' && *ext == '\0';
}

Image image_load(Device *device, const char *path) {
  if (!device || !path)
    return (Image){0};

  if (has_extension(path, "png")) {
    u8 *file = NULL;
    usize file_size = 0;
    DEPNG_ERROR err = depng_load_file(path, &file, &file_size);
    if (err != DEPNG_OK) {
      log_error("failed to read '%s': %s", path, depng_error_str(err));
      return (Image){0};
    }

    u8 *pixels = NULL;
    u32 w = 0, h = 0;
    err = depng_decode_rgba(file, file_size, &pixels, &w, &h);
    free(file);
    if (err != DEPNG_OK) {
      log_error("failed to decode '%s': %s", path, depng_error_str(err));
      return (Image){0};
    }

    Image img = image_create_from_data(
        device, pixels, (VkExtent3D){w, h, 1},
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, 1);
    free(pixels);

    if (img.image)
      log_trace("loaded image '%s' (%ux%u)", path, w, h);
    return img;
  }

  log_error("unsupported image format: '%s'", path);
  return (Image){0};
}
