#include "app/app.h"
#include "core/core.h"
#include "render/resource.h"
#include "core/window.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
  init();

  App *app = app_create("resource_test", VK_MAKE_API_VERSION(0, 0, 1, 0));
  assert(app);

  Window *window = window_create(app->instance, 64, 64, "resource_test");
  assert(window);

  Device *device = device_create(app->instance, window->surface);
  assert(device);

  ResourceManager *rm = create_resource_manager();
  assert(rm);

  Vertex tri[] = {
      {{0, 0, 0}, {0, 0, 1}, COLOR_WHITE, 0, 0},
      {{1, 0, 0}, {0, 0, 1}, COLOR_WHITE, 0, 0},
      {{0, 1, 0}, {0, 0, 1}, COLOR_WHITE, 0, 0},
  };
  u32 idx[] = {0, 1, 2};

  // Test 1: upload two meshes, handles are valid and distinct
  MeshHandle h1 = resource_upload_mesh(rm, device, tri, 3, idx, 3);
  assert(h1 != INVALID_HANDLE);

  MeshHandle h2 = resource_upload_mesh(rm, device, tri, 3, idx, 3);
  assert(h2 != INVALID_HANDLE);
  assert(h2 != h1);

  // Test 2: get by handle returns correct mesh
  Mesh *m = resource_get_mesh(rm, h1);
  assert(m != NULL);
  assert(m->vertexCount == 3);
  assert(m->indexCount == 3);

  // Test 3: invalid handle returns NULL
  assert(resource_get_mesh(rm, 999) == NULL);
  assert(resource_get_mesh(rm, INVALID_HANDLE) == NULL);

  // Test 4: free then get returns NULL
  resource_free_mesh(rm, device->device, h1);
  assert(resource_get_mesh(rm, h1) == NULL);

  // Test 5: freed slot is reused
  MeshHandle h3 = resource_upload_mesh(rm, device, tri, 3, idx, 3);
  assert(h3 == h1);

  // Test 6: double-free doesn't crash
  resource_free_mesh(rm, device->device, h2);
  resource_free_mesh(rm, device->device, h2);

  // Test 7: free invalid handle doesn't crash
  resource_free_mesh(rm, device->device, 999);
  resource_free_mesh(rm, device->device, INVALID_HANDLE);

  resource_manager_destroy(rm, device->device);
  device_destroy(device);
  window_destroy(window);
  app_destroy(app);
  terminate();

  printf("All resource manager tests passed!\n");
  return 0;
}
