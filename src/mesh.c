#include "mesh.h"
#include "logger.h"
#include <string.h>

Mesh vs_mesh_create(VsDevice *device, Vertex *vertices, u32 vertexCount,
                    u32 *indices, u32 indexCount) {
  Mesh mesh = {0};
  mesh.vertexCount = vertexCount;
  mesh.indexCount = indexCount;
  usize vertexBufferSize = sizeof(Vertex) * vertexCount;
  usize indexBufferSize = sizeof(u32) * indexCount;

  mesh.vertexBuffer = vs_create_buffer(vertexBufferSize, device,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!mesh.vertexBuffer.buffer) {
    vs_log_error("failed to create vertex buffer");
    return mesh;
  }

  mesh.indexBuffer = vs_create_buffer(indexBufferSize, device,
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!mesh.indexBuffer.buffer) {
    vs_log_error("failed to create index buffer");
    vs_destroy_buffer(&mesh.vertexBuffer, device->device);
    return (Mesh){0};
  }

  Buffer staging = vs_create_buffer(vertexBufferSize, device,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (!staging.buffer) {
    vs_log_error("failed to create vertex staging buffer");
    vs_destroy_buffer(&mesh.indexBuffer, device->device);
    vs_destroy_buffer(&mesh.vertexBuffer, device->device);
    return (Mesh){0};
  }
  void *data;
  vkMapMemory(device->device, staging.memory, 0, vertexBufferSize, 0, &data);
  memcpy(data, vertices, vertexBufferSize);
  vkUnmapMemory(device->device, staging.memory);
  vs_copy_buffer(device, staging.buffer, mesh.vertexBuffer.buffer,
                 vertexBufferSize);
  vs_destroy_buffer(&staging, device->device);

  Buffer stagingIdx = vs_create_buffer(
      indexBufferSize, device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (!stagingIdx.buffer) {
    vs_log_error("failed to create index staging buffer");
    vs_destroy_buffer(&mesh.indexBuffer, device->device);
    vs_destroy_buffer(&mesh.vertexBuffer, device->device);
    return (Mesh){0};
  }
  vkMapMemory(device->device, stagingIdx.memory, 0, indexBufferSize, 0, &data);
  memcpy(data, indices, indexBufferSize);
  vkUnmapMemory(device->device, stagingIdx.memory);
  vs_copy_buffer(device, stagingIdx.buffer, mesh.indexBuffer.buffer,
                 indexBufferSize);
  vs_destroy_buffer(&stagingIdx, device->device);

  return mesh;
}

void vs_mesh_destroy(Mesh *mesh, VkDevice device) {
  vs_log_trace("destroying mesh");
  if (!mesh || !mesh->vertexBuffer.buffer)
    return;
  vs_destroy_buffer(&mesh->vertexBuffer, device);
  vs_destroy_buffer(&mesh->indexBuffer, device);
}
