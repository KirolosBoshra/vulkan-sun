#ifndef RESOURCE_H
#define RESOURCE_H

#include "core/dynarr.h"
#include "device.h"
#include "types/image.h"
#include "types/mesh.h"
#include "types/types.h"
#include "utils/globals.h"
#include <vulkan/vulkan.h>

#define RES_LIST_INIT(res, H, T)                                               \
  do {                                                                         \
    DYNARR_INIT(&(res).list, T);                                               \
    DYNARR_INIT(&(res).active, bool);                                          \
    DYNARR_INIT(&(res).free, H);                                               \
  } while (0)

#define RES_LIST_DESTROY(res)                                                  \
  do {                                                                         \
    da_destroy(&(res).list);                                                   \
    da_destroy(&(res).active);                                                 \
    da_destroy(&(res).free);                                                   \
  } while (0)

#define INVALID_HANDLE UINT32_MAX

typedef u32 MeshHandle;
typedef u32 ImageHandle;

typedef struct {
  DynArr list;   // T
  DynArr active; // bool
  DynArr free;   // MeshHandle
} ResourceList;

typedef struct {
  ResourceList Meshes;
  ResourceList Images;
} ResourceManager;

ResourceManager *create_resource_manager(void);
void resource_manager_destroy(ResourceManager *rm, VkDevice device);

// Mesh management
MeshHandle resource_upload_mesh(ResourceManager *res, Device *device,
                                Vertex *verts, u32 vertCount, u32 *indices,
                                u32 indexCount);
void resource_free_mesh(ResourceManager *res, VkDevice device,
                        MeshHandle handle);
Mesh *resource_get_mesh(ResourceManager *res, MeshHandle handle);

// Image management
ImageHandle resource_create_image(ResourceManager *res, Device *device,
                                  VkExtent3D size, VkFormat format,
                                  u32 mipLevels);
ImageHandle resource_upload_image(ResourceManager *res, Device *device,
                                  void *data, VkExtent3D size, VkFormat format,
                                  u32 mipLevels);
void resource_free_image(ResourceManager *res, VkDevice device,
                         ImageHandle handle);
Image *resource_get_image(ResourceManager *res, ImageHandle handle);

static inline ImageHandle store_image(ResourceManager *res, Image img) {
  if (!img.image)
    return INVALID_HANDLE;

  ImageHandle handle;

  // if there is an empty space on the list
  if (res->Images.free.count > 0) {
    handle = *DYNARR_LAST(&res->Images.free, ImageHandle);
    da_pop(&res->Images.free);
  } else {
    handle = (ImageHandle)res->Images.list.count;
    if (!DYNARR_PUSH(&res->Images.list, Image))
      return INVALID_HANDLE;
    if (!DYNARR_PUSH(&res->Images.active, bool))
      return INVALID_HANDLE;
  }

  *DYNARR_GET(&res->Images.list, handle, Image) = img;
  *DYNARR_GET(&res->Images.active, handle, bool) = true;
  return handle;
}

#endif // RESOURCE_H
