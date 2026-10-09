#include "resource.h"
#include "core/dynarr.h"
#include "core/logger.h"
#include <stdbool.h>
#include <stdlib.h>

ResourceManager *create_resource_manager(void) {
  ResourceManager *rs = malloc(sizeof(ResourceManager));
  if (!rs) {
    log_error("failed to allocate resource manager");
    return NULL;
  }

  RES_LIST_INIT(rs->Meshes, MeshHandle, Mesh);
  RES_LIST_INIT(rs->Images, ImageHandle, Image);

  return rs;
}

MeshHandle resource_upload_mesh(ResourceManager *res, Device *device,
                                Vertex *verts, u32 vertCount, u32 *indices,
                                u32 indexCount) {
  Mesh mesh = mesh_create(device, verts, vertCount, indices, indexCount);

  if (!mesh.vertexBuffer.buffer)
    return INVALID_HANDLE;

  MeshHandle handle;

  // if there is an empty space on the list
  if (res->Meshes.free.count > 0) {
    MeshHandle *fh = DYNARR_LAST(&res->Meshes.free, MeshHandle);
    handle = *fh;
    da_pop(&res->Meshes.free);
  } else {
    handle = (MeshHandle)res->Meshes.list.count;
    if (!DYNARR_PUSH(&res->Meshes.list, Mesh))
      return INVALID_HANDLE;
    if (!DYNARR_PUSH(&res->Meshes.active, bool))
      return INVALID_HANDLE;
  }

  // Fill the slot
  *DYNARR_GET(&res->Meshes.list, handle, Mesh) = mesh;
  *DYNARR_GET(&res->Meshes.active, handle, bool) = true;

  return handle;
}

void resource_free_mesh(ResourceManager *res, VkDevice device,
                        MeshHandle handle) {
  if (handle >= res->Meshes.list.count) {
    log_error("invalid mesh handle: %u", handle);
    return;
  }

  bool *active = DYNARR_GET(&res->Meshes.active, handle, bool);
  if (!active || !*active) {
    log_error("mesh handle %u is not active", handle);
    return;
  }
  *active = false;
  mesh_destroy(DYNARR_GET(&res->Meshes.list, handle, Mesh), device);
  *DYNARR_PUSH(&res->Meshes.free, MeshHandle) = handle;
}

Mesh *resource_get_mesh(ResourceManager *res, MeshHandle handle) {
  if (handle >= res->Meshes.list.count)
    return NULL;

  bool *active = DYNARR_GET(&res->Meshes.active, handle, bool);
  if (!active || !*active)
    return NULL;

  return DYNARR_GET(&res->Meshes.list, handle, Mesh);
}

ImageHandle resource_create_image(ResourceManager *res, Device *device,
                                  VkExtent3D size, VkFormat format,
                                  u32 mipLevels) {
  Image img = image_create(device, size, format, VK_IMAGE_USAGE_SAMPLED_BIT,
                           VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
  return store_image(res, img);
}

ImageHandle resource_upload_image(ResourceManager *res, Device *device,
                                  void *data, VkExtent3D size, VkFormat format,
                                  u32 mipLevels) {
  Image img = image_create_from_data(device, data, size, format,
                                     VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
  return store_image(res, img);
}

void resource_free_image(ResourceManager *res, VkDevice device,
                         ImageHandle handle) {
  if (res->Images.list.count < handle) {
    log_error("invalid image handle: %u", handle);
    return;
  }

  bool *active = DYNARR_GET(&res->Images.active, handle, bool);
  if (!active || !*active) {
    log_error("image handle %u is not active", handle);
    return;
  }
  *active = false;
  image_destroy(DYNARR_GET(&res->Images.list, handle, Image), device);
  *DYNARR_PUSH(&res->Images.free, ImageHandle) = handle;
}

Image *resource_get_image(ResourceManager *res, ImageHandle handle) {
  if (handle >= res->Images.list.count)
    return NULL;

  bool *active = DYNARR_GET(&res->Images.active, handle, bool);
  if (!active || !*active)
    return NULL;

  return DYNARR_GET(&res->Images.list, handle, Image);
}

void resource_manager_destroy(ResourceManager *rm, VkDevice device) {
  if (!rm)
    return;

  // TODO: write a macro to iterate over active resources and destroy them

  // Iterate only active meshes
  DYNARR_FOREACH_INDEX(&rm->Meshes.active, bool, i) {
    bool *active = DYNARR_GET(&rm->Meshes.active, i, bool);

    if (active && *active) {
      Mesh *mesh = DYNARR_GET(&rm->Meshes.list, i, Mesh);
      if (mesh)
        mesh_destroy(mesh, device);
    }
  }

  // Iterate only active images
  DYNARR_FOREACH_INDEX(&rm->Images.active, bool, i) {
    bool *active = DYNARR_GET(&rm->Images.active, i, bool);
    if (active && *active) {
      Image *image = DYNARR_GET(&rm->Images.list, i, Image);
      if (image)
        image_destroy(image, device);
    }
  }

  RES_LIST_DESTROY(rm->Meshes);
  RES_LIST_DESTROY(rm->Images);
  free(rm);
}
