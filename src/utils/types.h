#ifndef TYPES_H
#define TYPES_H

#include "color.h"
#include "device.h"
#include "globals.h"
#include "utils/math.h"
#include <vulkan/vulkan.h>

typedef struct {
  Vec3 position;
  Vec3 normal;
  VsColor color;
  f32 u, v;
} Vertex;

typedef struct {
  VkBuffer buffer;
  VkDeviceMemory memory;
  usize size;
} Buffer;

typedef struct {
  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
} DepthImage;

Buffer vs_create_buffer(usize size, VsDevice *device, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties);

void vs_destroy_buffer(Buffer *buffer, VkDevice device);

void vs_copy_buffer(VsDevice *device, VkBuffer src, VkBuffer dst, usize size);

u32 vs_find_memory_type(VkPhysicalDevice physical, u32 typeBits,
                        VkMemoryPropertyFlags props);

DepthImage vs_create_depth_image(VkDevice device, VkPhysicalDevice physical,
                                 VkExtent2D extent);
void vs_destroy_depth_image(VkDevice device, DepthImage *di);

#endif
