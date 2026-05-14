#ifndef MATH_H
#define MATH_H

#include "globals.h"
#include <math.h>

#define PI 3.14159265358979323846f

#define MAT4_IDENTITY                                                          \
  ((Mat4){.m = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}})

// types

typedef struct {
  f32 x, y;
} Vec2;

typedef struct {
  f32 x, y, z;
} Vec3;

typedef struct {
  f32 x, y, z, w;
} Vec4;

typedef struct {
  // column-major order
  // m[col][row]
  f32 m[4][4];
} Mat4;

typedef struct {
  f32 x, y, z, w;
} Quat;

// Vec2 operations

static inline f32 vec2_dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

static inline f32 vec2_cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }

static inline Vec2 vec2_add(Vec2 a, Vec2 b) {
  return (Vec2){a.x + b.x, a.y + b.y};
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b) {
  return (Vec2){a.x - b.x, a.y - b.y};
}

static inline Vec2 vec2_mul(Vec2 a, Vec2 b) {
  return (Vec2){a.x * b.x, a.y * b.y};
}

static inline Vec2 vec2_scale(Vec2 v, f32 s) {
  return (Vec2){v.x * s, v.y * s};
}

static inline Vec2 vec2_normalize(Vec2 v) {
  f32 len = sqrtf(v.x * v.x + v.y * v.y);
  if (len < 1e-8f) return (Vec2){0, 0};
  f32 inv = 1.0f / len;
  return (Vec2){v.x * inv, v.y * inv};
}

static inline f32 vec2_length(Vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }

static inline f32 vec2_distance(Vec2 a, Vec2 b) {
  f32 dx = a.x - b.x;
  f32 dy = a.y - b.y;
  return sqrtf(dx * dx + dy * dy);
}

static inline Vec2 vec2_lerp(Vec2 a, Vec2 b, f32 t) {
  return (Vec2){a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

// Vec3 operations

static inline f32 vec3_dot(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
  return (Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x};
}

static inline Vec3 vec3_add(Vec3 a, Vec3 b) {
  return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b) {
  return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3 vec3_mul(Vec3 a, Vec3 b) {
  return (Vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

static inline Vec3 vec3_scale(Vec3 v, f32 s) {
  return (Vec3){v.x * s, v.y * s, v.z * s};
}

static inline Vec3 vec3_negate(Vec3 v) { return (Vec3){-v.x, -v.y, -v.z}; }

static inline Vec3 vec3_normalize(Vec3 v) {
  f32 len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len < 1e-8f) return (Vec3){0, 0, 0};
  f32 inv = 1.0f / len;
  return (Vec3){v.x * inv, v.y * inv, v.z * inv};
}

static inline f32 vec3_length(Vec3 v) {
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline f32 vec3_distance(Vec3 a, Vec3 b) {
  f32 dx = a.x - b.x;
  f32 dy = a.y - b.y;
  f32 dz = a.z - b.z;
  return sqrtf(dx * dx + dy * dy + dz * dz);
}

static inline Vec3 vec3_lerp(Vec3 a, Vec3 b, f32 t) {
  return (Vec3){a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t};
}

// Vec4 operations

static inline f32 vec4_dot(Vec4 a, Vec4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static inline Vec4 vec4_add(Vec4 a, Vec4 b) {
  return (Vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

static inline Vec4 vec4_sub(Vec4 a, Vec4 b) {
  return (Vec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

static inline Vec4 vec4_mul(Vec4 a, Vec4 b) {
  return (Vec4){a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

static inline Vec4 vec4_scale(Vec4 v, f32 s) {
  return (Vec4){v.x * s, v.y * s, v.z * s, v.w * s};
}

static inline Vec4 vec4_negate(Vec4 v) {
  return (Vec4){-v.x, -v.y, -v.z, -v.w};
}

static inline f32 vec4_length(Vec4 v) {
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

static inline Vec4 vec4_normalize(Vec4 v) {
  f32 len = vec4_length(v);
  if (len < 1e-8f) return (Vec4){0, 0, 0, 0};
  f32 inv = 1.0f / len;
  return (Vec4){v.x * inv, v.y * inv, v.z * inv, v.w * inv};
}

static inline Vec4 vec4_lerp(Vec4 a, Vec4 b, f32 t) {
  return (Vec4){a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t};
}

// homogeneous divide
static inline Vec3 vec4_to_vec3(Vec4 v) {
  if (v.w == 0.0f) return (Vec3){0, 0, 0};
  f32 inv = 1.0f / v.w;
  return (Vec3){v.x * inv, v.y * inv, v.z * inv};
}

// Quat operations

static inline Quat quat_from_euler(Vec3 euler_rad) {
  f32 cx = cosf(euler_rad.x * 0.5f);
  f32 sx = sinf(euler_rad.x * 0.5f);
  f32 cy = cosf(euler_rad.y * 0.5f);
  f32 sy = sinf(euler_rad.y * 0.5f);
  f32 cz = cosf(euler_rad.z * 0.5f);
  f32 sz = sinf(euler_rad.z * 0.5f);

  Quat q;
  q.w = cx * cy * cz - sx * sy * sz;
  q.x = sx * cy * cz + cx * sy * sz;
  q.y = cx * sy * cz - sx * cy * sz;
  q.z = cx * cy * sz + sx * sy * cz;
  return q;
}

static inline Quat quat_mul(Quat a, Quat b) {
  Quat q;
  q.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
  q.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
  q.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
  q.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
  return q;
}

static inline Quat quat_conjugate(Quat q) {
  return (Quat){-q.x, -q.y, -q.z, q.w};
}

static inline Quat quat_normalize(Quat q) {
  f32 len = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
  return (Quat){q.x / len, q.y / len, q.z / len, q.w / len};
}

static inline Quat quat_slerp(Quat a, Quat b, f32 t) {
  f32 cos_half_theta = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
  cos_half_theta = fmaxf(-1.0f, fminf(1.0f, cos_half_theta));

  if (cos_half_theta >= 1.0f) {
    f32 ta = 1.0f - t;
    return quat_normalize((Quat){
        a.x * ta + b.x * t, a.y * ta + b.y * t,
        a.z * ta + b.z * t, a.w * ta + b.w * t});
  }

  if (cos_half_theta < 0.0f) {
    b = (Quat){-b.x, -b.y, -b.z, -b.w};
    cos_half_theta = -cos_half_theta;
  }

  f32 half_theta = acosf(cos_half_theta);
  f32 sin_half_theta = sqrtf(1.0f - cos_half_theta * cos_half_theta);

  if (sin_half_theta < 0.001f) {
    f32 ta = 1.0f - t;
    return quat_normalize((Quat){
        a.x * ta + b.x * t, a.y * ta + b.y * t,
        a.z * ta + b.z * t, a.w * ta + b.w * t});
  }

  f32 ratio_a = sinf((1 - t) * half_theta) / sin_half_theta;
  f32 ratio_b = sinf(t * half_theta) / sin_half_theta;

  return quat_normalize(
      (Quat){a.x * ratio_a + b.x * ratio_b, a.y * ratio_a + b.y * ratio_b,
             a.z * ratio_a + b.z * ratio_b, a.w * ratio_a + b.w * ratio_b});
}

static inline Quat quat_from_axis_angle(Vec3 axis, f32 angle_rad) {
  f32 half_angle = angle_rad * 0.5f;
  f32 s = sinf(half_angle);
  f32 len = vec3_length(axis);
  return (Quat){axis.x / len * s, axis.y / len * s, axis.z / len * s, cosf(half_angle)};
}

static inline Mat4 quat_to_mat4(Quat q) {
  Mat4 m = MAT4_IDENTITY;
  f32 xx = q.x * q.x;
  f32 yy = q.y * q.y;
  f32 zz = q.z * q.z;
  f32 xy = q.x * q.y;
  f32 xz = q.x * q.z;
  f32 yz = q.y * q.z;
  f32 wx = q.w * q.x;
  f32 wy = q.w * q.y;
  f32 wz = q.w * q.z;

  m.m[0][0] = 1 - 2 * (yy + zz);
  m.m[0][1] = 2 * (xy + wz);
  m.m[0][2] = 2 * (xz - wy);

  m.m[1][0] = 2 * (xy - wz);
  m.m[1][1] = 1 - 2 * (xx + zz);
  m.m[1][2] = 2 * (yz + wx);

  m.m[2][0] = 2 * (xz + wy);
  m.m[2][1] = 2 * (yz - wx);
  m.m[2][2] = 1 - 2 * (xx + yy);

  return m;
}

// Mat4 operations

// fully unrolled matrix multiplication, I hate loops
// this ugly but no loops
static inline Mat4 mat4_mul(Mat4 a, Mat4 b) {
  Mat4 r;
  // col 0
  r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1] +
              a.m[2][0] * b.m[0][2] + a.m[3][0] * b.m[0][3];
  r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1] +
              a.m[2][1] * b.m[0][2] + a.m[3][1] * b.m[0][3];
  r.m[0][2] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[0][1] +
              a.m[2][2] * b.m[0][2] + a.m[3][2] * b.m[0][3];
  r.m[0][3] = a.m[0][3] * b.m[0][0] + a.m[1][3] * b.m[0][1] +
              a.m[2][3] * b.m[0][2] + a.m[3][3] * b.m[0][3];
  // col 1
  r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1] +
              a.m[2][0] * b.m[1][2] + a.m[3][0] * b.m[1][3];
  r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1] +
              a.m[2][1] * b.m[1][2] + a.m[3][1] * b.m[1][3];
  r.m[1][2] = a.m[0][2] * b.m[1][0] + a.m[1][2] * b.m[1][1] +
              a.m[2][2] * b.m[1][2] + a.m[3][2] * b.m[1][3];
  r.m[1][3] = a.m[0][3] * b.m[1][0] + a.m[1][3] * b.m[1][1] +
              a.m[2][3] * b.m[1][2] + a.m[3][3] * b.m[1][3];
  // col 2
  r.m[2][0] = a.m[0][0] * b.m[2][0] + a.m[1][0] * b.m[2][1] +
              a.m[2][0] * b.m[2][2] + a.m[3][0] * b.m[2][3];
  r.m[2][1] = a.m[0][1] * b.m[2][0] + a.m[1][1] * b.m[2][1] +
              a.m[2][1] * b.m[2][2] + a.m[3][1] * b.m[2][3];
  r.m[2][2] = a.m[0][2] * b.m[2][0] + a.m[1][2] * b.m[2][1] +
              a.m[2][2] * b.m[2][2] + a.m[3][2] * b.m[2][3];
  r.m[2][3] = a.m[0][3] * b.m[2][0] + a.m[1][3] * b.m[2][1] +
              a.m[2][3] * b.m[2][2] + a.m[3][3] * b.m[2][3];
  // col 3
  r.m[3][0] = a.m[0][0] * b.m[3][0] + a.m[1][0] * b.m[3][1] +
              a.m[2][0] * b.m[3][2] + a.m[3][0] * b.m[3][3];
  r.m[3][1] = a.m[0][1] * b.m[3][0] + a.m[1][1] * b.m[3][1] +
              a.m[2][1] * b.m[3][2] + a.m[3][1] * b.m[3][3];
  r.m[3][2] = a.m[0][2] * b.m[3][0] + a.m[1][2] * b.m[3][1] +
              a.m[2][2] * b.m[3][2] + a.m[3][2] * b.m[3][3];
  r.m[3][3] = a.m[0][3] * b.m[3][0] + a.m[1][3] * b.m[3][1] +
              a.m[2][3] * b.m[3][2] + a.m[3][3] * b.m[3][3];
  return r;
}

static Mat4 mat4_perspective(f32 fovy_rad, f32 aspect, f32 z_near, f32 z_far) {
  f32 tan_half = tanf(fovy_rad * 0.5f);
  Mat4 m = {0};
  m.m[0][0] = 1.0f / (aspect * tan_half);
  m.m[1][1] = -1.0f / tan_half;   // Y-flip for Vulkan
  m.m[2][2] = z_far / (z_near - z_far); // depth [0,1]
  m.m[2][3] = -1.0f;
  m.m[3][2] = -(z_far * z_near) / (z_far - z_near);
  return m;
}

static Mat4 mat4_translation(Vec3 t) {
  // returns a translation matrix for the given translation vector
  Mat4 m = MAT4_IDENTITY;
  m.m[3][0] = t.x;
  m.m[3][1] = t.y;
  m.m[3][2] = t.z;
  return m;
}

static Mat4 mat4_translate(Mat4 m, Vec3 t) {
  Mat4 trans = mat4_translation(t);
  return mat4_mul(trans, m);
}

static inline Mat4 mat4_scale(Mat4 m, Vec3 s) {
  Mat4 scale = {0};
  scale.m[0][0] = s.x;
  scale.m[1][1] = s.y;
  scale.m[2][2] = s.z;
  scale.m[3][3] = 1.0f;
  return mat4_mul(scale, m);
}

// no loops, just copy
static Mat4 mat4_transpose(Mat4 m) {
  Mat4 t;
  t.m[0][0] = m.m[0][0];
  t.m[0][1] = m.m[1][0];
  t.m[0][2] = m.m[2][0];
  t.m[0][3] = m.m[3][0];
  t.m[1][0] = m.m[0][1];
  t.m[1][1] = m.m[1][1];
  t.m[1][2] = m.m[2][1];
  t.m[1][3] = m.m[3][1];
  t.m[2][0] = m.m[0][2];
  t.m[2][1] = m.m[1][2];
  t.m[2][2] = m.m[2][2];
  t.m[2][3] = m.m[3][2];
  t.m[3][0] = m.m[0][3];
  t.m[3][1] = m.m[1][3];
  t.m[3][2] = m.m[2][3];
  t.m[3][3] = m.m[3][3];
  return t;
}

// Inverse of a rigid-body transform (rotation + translation, no scale)
static inline Mat4 mat4_inverse(Mat4 m) {
  Mat4 r = {0};

  // Upper-left 3x3 = R^T (transpose of rotation part)
  r.m[0][0] = m.m[0][0];
  r.m[0][1] = m.m[1][0];
  r.m[0][2] = m.m[2][0];
  r.m[1][0] = m.m[0][1];
  r.m[1][1] = m.m[1][1];
  r.m[1][2] = m.m[2][1];
  r.m[2][0] = m.m[0][2];
  r.m[2][1] = m.m[1][2];
  r.m[2][2] = m.m[2][2];

  // Translation column = -R^T * t
  r.m[3][0] =
      -(r.m[0][0] * m.m[3][0] + r.m[1][0] * m.m[3][1] + r.m[2][0] * m.m[3][2]);
  r.m[3][1] =
      -(r.m[0][1] * m.m[3][0] + r.m[1][1] * m.m[3][1] + r.m[2][1] * m.m[3][2]);
  r.m[3][2] =
      -(r.m[0][2] * m.m[3][0] + r.m[1][2] * m.m[3][1] + r.m[2][2] * m.m[3][2]);

  r.m[3][3] = 1.0f;
  return r;
}

static inline Mat4 mat4_rotate_y(Mat4 m, f32 r) {
  Mat4 rot = MAT4_IDENTITY;
  f32 c = cosf(r), s = sinf(r);
  rot.m[0][0] = c;
  rot.m[0][2] = -s;
  rot.m[2][0] = s;
  rot.m[2][2] = c;
  return mat4_mul(rot, m);
}

static inline Mat4 mat4_rotate_x(Mat4 m, f32 r) {
  Mat4 rot = MAT4_IDENTITY;
  f32 c = cosf(r), s = sinf(r);
  rot.m[1][1] = c;
  rot.m[1][2] = s;
  rot.m[2][1] = -s;
  rot.m[2][2] = c;
  return mat4_mul(rot, m);
}

static inline Mat4 mat4_rotate_z(Mat4 m, f32 r) {
  Mat4 rot = MAT4_IDENTITY;
  f32 c = cosf(r), s = sinf(r);
  rot.m[0][0] = c;
  rot.m[0][1] = s;
  rot.m[1][0] = -s;
  rot.m[1][1] = c;
  return mat4_mul(rot, m);
}

// unrolled XYZ matrix rotation
static inline Mat4 mat4_rotate(Mat4 m, Vec3 angles_rad) {
  f32 cx = cosf(angles_rad.x), sx = sinf(angles_rad.x);
  f32 cy = cosf(angles_rad.y), sy = sinf(angles_rad.y);
  f32 cz = cosf(angles_rad.z), sz = sinf(angles_rad.z);

  Mat4 rot = {0};
  // col 0
  rot.m[0][0] = cy * cz;
  rot.m[0][1] = sx * sy * cz + cx * sz;
  rot.m[0][2] = -cx * sy * cz + sx * sz;
  // col 1
  rot.m[1][0] = -cy * sz;
  rot.m[1][1] = -sx * sy * sz + cx * cz;
  rot.m[1][2] = cx * sy * sz + sx * cz;
  // col 2
  rot.m[2][0] = sy;
  rot.m[2][1] = -sx * cy;
  rot.m[2][2] = cx * cy;

  rot.m[3][3] = 1.0f;
  return mat4_mul(rot, m);
}

static inline Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
  Vec3 f = vec3_normalize(vec3_sub(center, eye));
  Vec3 r = vec3_normalize(vec3_cross(f, up));
  Vec3 u = vec3_cross(r, f);

  Mat4 m = {0};
  m.m[0][0] =  r.x; m.m[1][0] =  r.y; m.m[2][0] =  r.z;
  m.m[0][1] =  u.x; m.m[1][1] =  u.y; m.m[2][1] =  u.z;
  m.m[0][2] = -f.x; m.m[1][2] = -f.y; m.m[2][2] = -f.z;
  m.m[3][0] = -vec3_dot(r, eye);
  m.m[3][1] = -vec3_dot(u, eye);
  m.m[3][2] =  vec3_dot(f, eye);
  m.m[3][3] = 1.0f;
  return m;
}

static inline Vec4 mat4_mul_vec4(Mat4 m, Vec4 v) {
  return (Vec4){
      m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w,
      m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w,
      m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w,
      m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w,
  };
}

static inline Vec3 mat4_transform_point(Mat4 m, Vec3 p) {
  Vec4 r = mat4_mul_vec4(m, (Vec4){p.x, p.y, p.z, 1.0f});
  return (Vec3){r.x, r.y, r.z};
}

static inline Vec3 mat4_transform_dir(Mat4 m, Vec3 d) {
  Vec4 r = mat4_mul_vec4(m, (Vec4){d.x, d.y, d.z, 0.0f});
  return (Vec3){r.x, r.y, r.z};
}

static inline Mat4 mat4_ortho(f32 left, f32 right, f32 bottom, f32 top,
                               f32 z_near, f32 z_far) {
  Mat4 m = {0};
  m.m[0][0] =  2.0f / (right - left);
  m.m[1][1] = -2.0f / (top - bottom);
  m.m[2][2] = -1.0f / (z_far - z_near);
  m.m[3][0] = -(right + left)   / (right - left);
  m.m[3][1] =  (top   + bottom) / (top   - bottom);
  m.m[3][2] = -z_near / (z_far - z_near);
  m.m[3][3] =  1.0f;
  return m;
}

#endif // MATH_H
