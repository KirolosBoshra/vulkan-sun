#ifndef CAMERA_H
#define CAMERA_H

#include "utils/globals.h"
#include "utils/math.h"

typedef struct {
  Vec3 position;
  Vec3 target;
  Vec3 up;
  f32 fov;
  f32 aspect;
  f32 z_near;
  f32 z_far;
} Camera;

static inline Camera create_camera(i32 width, i32 height) {
  Camera c = {
      .position = {0, 0, 0.0f},
      .target = {0, 0, 0},
      .up = {0, 1, 0},
      .fov = PI / 4.0f,
      .aspect = (f32)width / (f32)height,
      .z_near = 0.1f,
      .z_far = 100.0f,
  };
  return c;
}

static inline Mat4 camera_view(Camera *c) {
  return mat4_look_at(c->position, c->target, c->up);
}

static inline Mat4 camera_proj(Camera *c) {
  return mat4_perspective(c->fov, c->aspect, c->z_near, c->z_far);
}

static inline Mat4 camera_vp(Camera *c) {
  return mat4_mul(camera_proj(c), camera_view(c));
}

#endif
