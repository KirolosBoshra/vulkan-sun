#ifndef PIPLINE_H
#define PIPLINE_H

#include "types/shader.h"
#include "utils/globals.h"
#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef struct {
  VkPipeline handle;
  VkPipelineLayout layout;
  VkShaderStageFlags pushStages;
  u32 pushSize;
} Pipeline;

Pipeline *pipeline_create(VkDevice device, VkFormat colorFormat,
                               Shader vert, Shader frag, u32 pushSize,
                               VkDescriptorSetLayout *setLayouts,
                               u32 setLayoutCount, bool depthEnable);
void pipeline_destroy(Pipeline *pipeline, VkDevice device);

#endif
