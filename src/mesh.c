#include "mesh.h"
#include "logger.h"
#include <string.h>

Mesh vs_mesh_create(VsDevice *device, Vertex *vertices, u32 vertexCount,
                    u32 *indices, u32 indexCount) {
  Mesh mesh;
  mesh.vertexCount = vertexCount;
  mesh.indexCount = indexCount;
  usize vertexBufferSize = sizeof(Vertex) * vertexCount;
  usize indexBufferSize = sizeof(u32) * indexCount;

  // Create vertex buffer
  mesh.vertexBuffer = vs_create_buffer(vertexBufferSize, device,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // Create index buffer
  mesh.indexBuffer = vs_create_buffer(indexBufferSize, device,
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  // Upload vertex data
  Buffer stagingVertexBuffer = vs_create_buffer(
      vertexBufferSize, device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  void *data;
  vkMapMemory(device->device, stagingVertexBuffer.memory, 0, vertexBufferSize,
              0, &data);
  memcpy(data, vertices, vertexBufferSize);
  vkUnmapMemory(device->device, stagingVertexBuffer.memory);
  vs_copy_buffer(device, stagingVertexBuffer.buffer, mesh.vertexBuffer.buffer,
                 vertexBufferSize);
  vs_destroy_buffer(&stagingVertexBuffer, device->device);

  // Upload index data
  Buffer stagingIndexBuffer = vs_create_buffer(
      indexBufferSize, device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  vkMapMemory(device->device, stagingIndexBuffer.memory, 0, indexBufferSize, 0,
              &data);
  memcpy(data, indices, indexBufferSize);
  vkUnmapMemory(device->device, stagingIndexBuffer.memory);
  vs_copy_buffer(device, stagingIndexBuffer.buffer, mesh.indexBuffer.buffer,
                 indexBufferSize);
  vs_destroy_buffer(&stagingIndexBuffer, device->device);

  return mesh;
}

void vs_mesh_destroy(Mesh *mesh, VkDevice device) {
  vs_log_trace("destroying mesh");
  vs_destroy_buffer(&mesh->vertexBuffer, device);
  vs_destroy_buffer(&mesh->indexBuffer, device);
}
