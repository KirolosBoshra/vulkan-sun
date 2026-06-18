#include "descriptor.h"
#include "core/logger.h"
#include <stdlib.h>

void desc_alloc_init(DescriptorAllocator *a, u32 initialSets,
                        PoolSizeRatio *ratios, u32 ratioCount) {
  a->ratioCount = ratioCount;
  a->ratios = malloc(ratioCount * sizeof(PoolSizeRatio));
  for (u32 i = 0; i < ratioCount; i++)
    a->ratios[i] = ratios[i];

  a->setsPerPool = initialSets;
  a->readyCount = 0;
  a->readyCap = 4;
  a->readyPools = malloc(a->readyCap * sizeof(VkDescriptorPool));

  a->fullCount = 0;
  a->fullCap = 4;
  a->fullPools = malloc(a->fullCap * sizeof(VkDescriptorPool));
}

void desc_alloc_destroy(DescriptorAllocator *a, VkDevice device) {
  for (u32 i = 0; i < a->readyCount; i++)
    vkDestroyDescriptorPool(device, a->readyPools[i], NULL);
  for (u32 i = 0; i < a->fullCount; i++)
    vkDestroyDescriptorPool(device, a->fullPools[i], NULL);
  free(a->readyPools);
  free(a->fullPools);
  free(a->ratios);
}

void desc_alloc_clear(DescriptorAllocator *a, VkDevice device) {
  for (u32 i = 0; i < a->readyCount; i++)
    vkResetDescriptorPool(device, a->readyPools[i], 0);
  for (u32 i = 0; i < a->fullCount; i++) {
    vkResetDescriptorPool(device, a->fullPools[i], 0);
    if (a->readyCount >= a->readyCap) {
      a->readyCap *= 2;
      a->readyPools =
          realloc(a->readyPools, a->readyCap * sizeof(VkDescriptorPool));
    }
    a->readyPools[a->readyCount++] = a->fullPools[i];
  }
  a->fullCount = 0;
}

static VkDescriptorPool create_pool(VkDevice device, u32 setCount,
                                    PoolSizeRatio *ratios, u32 ratioCount) {
  VkDescriptorPoolSize *sizes =
      malloc(ratioCount * sizeof(VkDescriptorPoolSize));
  for (u32 i = 0; i < ratioCount; i++)
    sizes[i] = (VkDescriptorPoolSize){
        .type = ratios[i].type,
        .descriptorCount = (u32)(ratios[i].ratio * (f32)setCount),
    };

  VkDescriptorPoolCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = setCount,
      .poolSizeCount = ratioCount,
      .pPoolSizes = sizes,
  };

  VkDescriptorPool pool;
  if (vkCreateDescriptorPool(device, &info, NULL, &pool) != VK_SUCCESS) {
    log_error("failed to create descriptor pool");
    pool = VK_NULL_HANDLE;
  }
  free(sizes);
  return pool;
}

static VkDescriptorPool get_pool(DescriptorAllocator *a, VkDevice device) {
  if (a->readyCount > 0) {
    VkDescriptorPool p = a->readyPools[--a->readyCount];
    return p;
  }

  VkDescriptorPool p =
      create_pool(device, a->setsPerPool, a->ratios, a->ratioCount);
  a->setsPerPool = (u32)((f32)a->setsPerPool * 1.5f);
  if (a->setsPerPool > 4092)
    a->setsPerPool = 4092;
  return p;
}

VkDescriptorSet desc_alloc_allocate(DescriptorAllocator *a,
                                       VkDevice device,
                                       VkDescriptorSetLayout layout) {
  VkDescriptorPool pool = get_pool(a, device);

  VkDescriptorSetAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &layout,
  };

  VkDescriptorSet set;
  VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &set);

  if (result == VK_ERROR_OUT_OF_POOL_MEMORY ||
      result == VK_ERROR_FRAGMENTED_POOL) {
    if (a->fullCount >= a->fullCap) {
      a->fullCap *= 2;
      a->fullPools =
          realloc(a->fullPools, a->fullCap * sizeof(VkDescriptorPool));
    }
    a->fullPools[a->fullCount++] = pool;

    pool = get_pool(a, device);
    allocInfo.descriptorPool = pool;

    if (vkAllocateDescriptorSets(device, &allocInfo, &set) != VK_SUCCESS) {
      log_error("descriptor allocation failed after pool retry");
      return VK_NULL_HANDLE;
    }
  }

  if (a->readyCount >= a->readyCap) {
    a->readyCap *= 2;
    a->readyPools =
        realloc(a->readyPools, a->readyCap * sizeof(VkDescriptorPool));
  }
  a->readyPools[a->readyCount++] = pool;
  return set;
}

void desc_layout_init(DescriptorLayoutBuilder *b) {
  b->count = 0;
  b->binds = NULL;
}

void desc_layout_add(DescriptorLayoutBuilder *b, u32 binding,
                        VkDescriptorType type, VkShaderStageFlags stage) {
  u32 idx = b->count++;
  b->binds = realloc(b->binds, b->count * sizeof(VkDescriptorSetLayoutBinding));
  b->binds[idx] = (VkDescriptorSetLayoutBinding){
      .binding = binding,
      .descriptorType = type,
      .descriptorCount = 1,
      .stageFlags = stage,
  };
}

VkDescriptorSetLayout desc_layout_build(DescriptorLayoutBuilder *b,
                                           VkDevice device) {
  VkDescriptorSetLayoutCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = b->count,
      .pBindings = b->binds,
  };

  VkDescriptorSetLayout layout;
  if (vkCreateDescriptorSetLayout(device, &info, NULL, &layout) != VK_SUCCESS) {
    log_error("failed to create descriptor set layout");
    return VK_NULL_HANDLE;
  }
  return layout;
}

void desc_layout_cleanup(DescriptorLayoutBuilder *b) {
  free(b->binds);
  b->binds = NULL;
  b->count = 0;
}

void desc_writer_init(DescriptorWriter *w,
                          VkDescriptorBufferInfo *bufInfos,
                          VkDescriptorImageInfo *imgInfos,
                          VkWriteDescriptorSet *writes, u32 maxWrites) {
  w->bufferInfos = bufInfos;
  w->imageInfos = imgInfos;
  w->writes = writes;
  w->writeCount = 0;
  w->bufferCount = 0;
  w->imageCount = 0;
  w->maxWrites = maxWrites;
}

void desc_writer_write_buffer(DescriptorWriter *w, u32 binding,
                                  VkBuffer buffer, usize size, usize offset,
                                  VkDescriptorType type) {
  if (w->writeCount >= w->maxWrites) return;
  VkDescriptorBufferInfo *info = &w->bufferInfos[w->bufferCount];
  *info = (VkDescriptorBufferInfo){.buffer = buffer, .offset = offset, .range = size};

  VkWriteDescriptorSet *write = &w->writes[w->writeCount];
  *write = (VkWriteDescriptorSet){
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = NULL,
      .dstBinding = binding,
      .dstSet = VK_NULL_HANDLE,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = type,
      .pBufferInfo = info,
      .pImageInfo = NULL,
      .pTexelBufferView = NULL,
  };

  w->writeCount++;
  w->bufferCount++;
}

void desc_writer_write_image(DescriptorWriter *w, u32 binding,
                                 VkImageView imageView, VkSampler sampler,
                                 VkImageLayout layout, VkDescriptorType type) {
  if (w->writeCount >= w->maxWrites) return;
  VkDescriptorImageInfo *info = &w->imageInfos[w->imageCount];
  *info = (VkDescriptorImageInfo){
      .sampler = sampler, .imageView = imageView, .imageLayout = layout};

  VkWriteDescriptorSet *write = &w->writes[w->writeCount];
  *write = (VkWriteDescriptorSet){
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = NULL,
      .dstBinding = binding,
      .dstSet = VK_NULL_HANDLE,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = type,
      .pImageInfo = info,
      .pBufferInfo = NULL,
      .pTexelBufferView = NULL,
  };

  w->writeCount++;
  w->imageCount++;
}

void desc_writer_update(DescriptorWriter *w, VkDevice device,
                           VkDescriptorSet set) {
  for (u32 i = 0; i < w->writeCount; i++)
    w->writes[i].dstSet = set;

  vkUpdateDescriptorSets(device, w->writeCount, w->writes, 0, NULL);
}
