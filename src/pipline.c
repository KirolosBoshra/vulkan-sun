#include "pipline.h"
#include "logger.h"
#include "utils/types.h"
#include <stddef.h>
#include <stdlib.h>

VsPipeline *vs_pipeline_create(VkDevice device, VkFormat colorFormat,
                               VsShader vert, VsShader frag, u32 pushSize) {
  vs_log_trace("creating graphics pipeline");

  VsPipeline *pipeline = malloc(sizeof(VsPipeline));
  if (!pipeline) {
    vs_log_error("failed to allocate pipeline");
    return NULL;
  }

  if (!vert.module || !frag.module) {
    vs_log_error("invalid shader modules");
    free(pipeline);
    return NULL;
  }

  pipeline->pushSize = pushSize;

  VkPipelineShaderStageCreateInfo stages[2] = {
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = vert.stage,
       .module = vert.module,
       .pName = "main"},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = frag.stage,
       .module = frag.module,
       .pName = "main"},
  };

  VkVertexInputBindingDescription binding = {
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };

  VkVertexInputAttributeDescription attrs[4] = {
      {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = offsetof(Vertex, position)},
      {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT,
       .offset = offsetof(Vertex, normal)},
      {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT,
       .offset = offsetof(Vertex, color)},
      {.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT,
       .offset = offsetof(Vertex, u)},
  };

  VkPipelineVertexInputStateCreateInfo vertexInput = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &binding,
      .vertexAttributeDescriptionCount = 4,
      .pVertexAttributeDescriptions = attrs,
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  };

  VkViewport viewport = {0, 0, 1, 1, 0, 1};
  VkRect2D scissor = {{0, 0}, {1, 1}};
  VkPipelineViewportStateCreateInfo viewportState = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports = &viewport,
      .scissorCount = 1,
      .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  };

  VkPipelineColorBlendAttachmentState blendAttach = {
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &blendAttach,
  };

  const VkDynamicState dynamics[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
  };

  VkPipelineDynamicStateCreateInfo dynamic = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates = dynamics,
  };

  VkPushConstantRange pushRange = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = pushSize,
  };

  VkPipelineLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pushConstantRangeCount = pushSize > 0 ? 1 : 0,
      .pPushConstantRanges = pushSize > 0 ? &pushRange : NULL,
  };

  if (vkCreatePipelineLayout(device, &layoutInfo, NULL, &pipeline->layout) !=
      VK_SUCCESS) {
    vs_log_error("failed to create pipeline layout");
    free(pipeline);
    return NULL;
  }

  VkPipelineRenderingCreateInfo rendering = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &colorFormat,
  };

  VkGraphicsPipelineCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &rendering,
      .stageCount = 2,
      .pStages = stages,
      .pVertexInputState = &vertexInput,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pColorBlendState = &blending,
      .pDynamicState = &dynamic,
      .layout = pipeline->layout,
      .renderPass = VK_NULL_HANDLE,
  };

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, NULL,
                                &pipeline->handle) != VK_SUCCESS) {
    vs_log_error("failed to create graphics pipeline");
    vkDestroyPipelineLayout(device, pipeline->layout, NULL);
    free(pipeline);
    return NULL;
  }

  vs_log_trace("pipeline created");
  return pipeline;
}

void vs_pipeline_destroy(VsPipeline *pipeline, VkDevice device) {
  vs_log_trace("destroying pipeline");
  if (!pipeline)
    return;
  vkDestroyPipeline(device, pipeline->handle, NULL);
  vkDestroyPipelineLayout(device, pipeline->layout, NULL);
  free(pipeline);
}
