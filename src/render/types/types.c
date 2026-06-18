#include "types.h"
#include "core/logger.h"

u32 find_memory_type(VkPhysicalDevice physical, u32 typeBits,
                        VkMemoryPropertyFlags props) {
  VkPhysicalDeviceMemoryProperties mem;
  vkGetPhysicalDeviceMemoryProperties(physical, &mem);
  for (u32 i = 0; i < mem.memoryTypeCount; i++)
    if ((typeBits & (1 << i)) &&
        (mem.memoryTypes[i].propertyFlags & props) == props)
      return i;
  return UINT32_MAX;
}

Buffer create_buffer(usize size, Device *device, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties) {
  Buffer buffer;

  // Create the buffer
  VkBufferCreateInfo bufferInfo = {0};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device->device, &bufferInfo, NULL, &buffer.buffer) !=
      VK_SUCCESS) {
    log_error("Failed to create buffer!");
    return (Buffer){0};
  }

  // Get memory requirements
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device->device, buffer.buffer,
                                &memRequirements);

  // Allocate memory
  VkMemoryAllocateInfo allocInfo = {0};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = find_memory_type(
      device->physical, memRequirements.memoryTypeBits, properties);
  if (vkAllocateMemory(device->device, &allocInfo, NULL, &buffer.memory) !=
      VK_SUCCESS) {
    log_error("Failed to allocate buffer memory!");
    vkDestroyBuffer(device->device, buffer.buffer, NULL);
    return (Buffer){0};
  }

  // Bind memory to the buffer
  vkBindBufferMemory(device->device, buffer.buffer, buffer.memory, 0);
  buffer.size = size;

  return buffer;
}

void destroy_buffer(Buffer *buffer, VkDevice device) {
  vkDestroyBuffer(device, buffer->buffer, NULL);
  vkFreeMemory(device, buffer->memory, NULL);
}

// <TODO> This is a bit of a hack, we should really be using command buffers
// and queues
void copy_buffer(Device *device, VkBuffer src, VkBuffer dst, usize size) {
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
  vkCmdCopyBuffer(cmd, src, dst, 1, &(VkBufferCopy){.size = size});
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
}

DepthImage create_depth_image(VkDevice device, VkPhysicalDevice physical,
                                 VkExtent2D extent) {
  DepthImage di = {0};

  VkImageCreateInfo imageInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = VK_FORMAT_D32_SFLOAT,
      .extent = {.width = extent.width, .height = extent.height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  if (vkCreateImage(device, &imageInfo, NULL, &di.image) != VK_SUCCESS) {
    log_error("failed to create depth image");
    return di;
  }

  VkMemoryRequirements reqs;
  vkGetImageMemoryRequirements(device, di.image, &reqs);

  VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = reqs.size,
      .memoryTypeIndex = find_memory_type(
          physical, reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
  };

  if (vkAllocateMemory(device, &allocInfo, NULL, &di.memory) != VK_SUCCESS) {
    log_error("failed to allocate depth memory");
    vkDestroyImage(device, di.image, NULL);
    return (DepthImage){0};
  }

  vkBindImageMemory(device, di.image, di.memory, 0);

  VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = di.image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = VK_FORMAT_D32_SFLOAT,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  if (vkCreateImageView(device, &viewInfo, NULL, &di.view) != VK_SUCCESS) {
    log_error("failed to create depth image view");
    vkFreeMemory(device, di.memory, NULL);
    vkDestroyImage(device, di.image, NULL);
    return (DepthImage){0};
  }

  return di;
}

void destroy_depth_image(VkDevice device, DepthImage *di) {
  if (!di || !di->image)
    return;
  vkDestroyImageView(device, di->view, NULL);
  vkFreeMemory(device, di->memory, NULL);
  vkDestroyImage(device, di->image, NULL);
}
