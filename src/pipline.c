#include "pipline.h"
#include <stdio.h>
#include <stdlib.h>

VsPipeline *vs_pipeline_create(VkDevice device, VkFormat colorFormat,
                               VsShader vert, VsShader frag) {
  VsPipeline *pipeline = malloc(sizeof(VsPipeline));
  if (!pipeline) {
    fprintf(stderr, "failed to allocate pipeline\n");
    return NULL;
  }

  if (!vert.module || !frag.module) {
    fprintf(stderr, "invalid shader modules\n");
    free(pipeline);
    return NULL;
  }

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

  VkPipelineVertexInputStateCreateInfo vertexInput = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0,
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

  VkPipelineLayoutCreateInfo layoutInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  };

  if (vkCreatePipelineLayout(device, &layoutInfo, NULL, &pipeline->layout) !=
      VK_SUCCESS) {
    fprintf(stderr, "failed to create pipeline layout\n");
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
    fprintf(stderr, "failed to create graphics pipeline\n");
    vkDestroyPipelineLayout(device, pipeline->layout, NULL);
    free(pipeline);
    return NULL;
  }

  return pipeline;
}

void vs_pipeline_destroy(VsPipeline *pipeline, VkDevice device) {
  if (!pipeline)
    return;
  vkDestroyPipeline(device, pipeline->handle, NULL);
  vkDestroyPipelineLayout(device, pipeline->layout, NULL);
  free(pipeline);
}
