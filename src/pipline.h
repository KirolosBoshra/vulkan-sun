#ifndef PIPLINE_H
#define PIPLINE_H

#include "shader.h"
#include "utils/globals.h"
#include <vulkan/vulkan.h>

typedef struct {
  VkPipeline handle;
  VkPipelineLayout layout;
  u32 pushSize;
} VsPipeline;

VsPipeline *vs_pipeline_create(VkDevice device, VkFormat colorFormat,
                               VsShader vert, VsShader frag, u32 pushSize);
void vs_pipeline_destroy(VsPipeline *pipeline, VkDevice device);

#endif
