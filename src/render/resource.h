#ifndef RESOURCE_H
#define RESOURCE_H

#include "core/dynarr.h"
#include "device.h"
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
// typedef u32 TextureHandle;

typedef struct {
  DynArr list;   // T
  DynArr active; // bool
  DynArr free;   // MeshHandle
} ResourceList;

typedef struct {
  ResourceList Meshes;
  // ResourceList Textures;
} ResourceManager;

ResourceManager *create_resource_manager(void);
void resource_manager_destroy(ResourceManager *rm, VkDevice device);

MeshHandle resource_upload_mesh(ResourceManager *res, Device *device,
                                Vertex *verts, u32 vertCount, u32 *indices,
                                u32 indexCount);
void resource_free_mesh(ResourceManager *res, VkDevice device,
                        MeshHandle handle);
Mesh *resource_get_mesh(ResourceManager *res, MeshHandle handle);

#endif // RESOURCE_H
