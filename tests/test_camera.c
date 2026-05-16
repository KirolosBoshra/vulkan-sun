#include "camera.h"
#include "utils/math.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static int near(f32 a, f32 b, f32 eps) { return fabsf(a - b) < eps; }

static void test_camera_proj_near_far(void) {
  Camera c = {0};
  c.fov = PI / 2.0f;
  c.aspect = 1.0f;
  c.z_near = 1.0f;
  c.z_far = 2.0f;
  Mat4 p = camera_proj(&c);
  // near plane (z=-1) → depth 0
  f32 z_near = (-1.0f) * p.m[2][2] + 1 * p.m[3][2];
  f32 w_near = (-1.0f) * p.m[2][3] + 1 * p.m[3][3];
  assert(near(z_near / w_near, 0.0f, 0.001f));
  // far plane (z=-2) → depth 1
  f32 z_far = (-2.0f) * p.m[2][2] + 1 * p.m[3][2];
  f32 w_far = (-2.0f) * p.m[2][3] + 1 * p.m[3][3];
  assert(near(z_far / w_far, 1.0f, 0.001f));
}

static void test_camera_view_translates(void) {
  Camera c = {0};
  c.position = (Vec3){0, 0, 5};
  c.target = (Vec3){0, 0, 0};
  c.up = (Vec3){0, 1, 0};
  Mat4 v = camera_view(&c);
  // point at origin in world → should be at (0, 0, -5) in view space
  Vec4 p = mat4_mul_vec4(v, (Vec4){0, 0, 0, 1});
  assert(near(p.z, -5.0f, 0.001f));
  assert(near(p.x, 0.0f, 0.001f));
  assert(near(p.y, 0.0f, 0.001f));
}

static void test_camera_vp_transform(void) {
  Camera c = {0};
  c.position = (Vec3){0, 0, 5};
  c.target = (Vec3){0, 0, 0};
  c.up = (Vec3){0, 1, 0};
  c.fov = PI / 2.0f;
  c.aspect = 1.0f;
  c.z_near = 1.0f;
  c.z_far = 100.0f;
  Mat4 vp = camera_vp(&c);
  Vec4 clip = mat4_mul_vec4(vp, (Vec4){1, 0, 0, 1});
  assert(fabsf(clip.x) > 0);
  assert(fabsf(clip.w) > 0);
}

static void test_camera_rotate_view(void) {
  Camera c = {0};
  c.position = (Vec3){5, 0, 0};
  c.target = (Vec3){0, 0, 0};
  c.up = (Vec3){0, 1, 0};
  Mat4 v = camera_view(&c);
  Vec4 p = mat4_mul_vec4(v, (Vec4){0, 0, 0, 1});
  assert(near(p.z, -5.0f, 0.001f));
}

static void test_create_camera_defaults(void) {
  Camera c = create_camera(600, 400);
  assert(near(c.fov, PI / 4.0f, 0.001f));
  assert(near(c.aspect, 1.5f, 0.001f));
  assert(near(c.z_near, 0.1f, 0.001f));
  assert(near(c.z_far, 100.0f, 0.001f));
}

int main(void) {
  test_create_camera_defaults();
  test_camera_proj_near_far();
  test_camera_view_translates();
  test_camera_vp_transform();
  test_camera_rotate_view();
  printf("All camera tests passed!\n");
  return 0;
}
