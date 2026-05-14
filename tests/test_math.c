#include "utils/math.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static int is_near(f32 a, f32 b, f32 eps) { return fabsf(a - b) < eps; }

static int mat4_eq(Mat4 a, Mat4 b, f32 eps) {
  for (i32 c = 0; c < 4; c++)
    for (i32 r = 0; r < 4; r++)
      if (!is_near(a.m[c][r], b.m[c][r], eps))
        return 0;
  return 1;
}

static void test_vec2_dot(void) {
  assert(is_near(vec2_dot((Vec2){1, 0}, (Vec2){0, 1}), 0, 0.001f));
  assert(is_near(vec2_dot((Vec2){3, 4}, (Vec2){1, 0}), 3, 0.001f));
  assert(is_near(vec2_dot((Vec2){2, 3}, (Vec2){4, 5}), 23, 0.001f));
}

static void test_vec2_cross(void) {
  assert(is_near(vec2_cross((Vec2){1, 0}, (Vec2){0, 1}), 1, 0.001f));
  assert(is_near(vec2_cross((Vec2){0, 1}, (Vec2){1, 0}), -1, 0.001f));
  assert(is_near(vec2_cross((Vec2){2, 3}, (Vec2){5, 7}), 2*7 - 3*5, 0.001f));
}

static void test_vec2_add(void) {
  Vec2 r = vec2_add((Vec2){1, 2}, (Vec2){3, 4});
  assert(is_near(r.x, 4, 0.001f));
  assert(is_near(r.y, 6, 0.001f));
}

static void test_vec2_sub(void) {
  Vec2 r = vec2_sub((Vec2){5, 8}, (Vec2){3, 2});
  assert(is_near(r.x, 2, 0.001f));
  assert(is_near(r.y, 6, 0.001f));
}

static void test_vec2_mul(void) {
  Vec2 r = vec2_mul((Vec2){2, 3}, (Vec2){4, 5});
  assert(is_near(r.x, 8, 0.001f));
  assert(is_near(r.y, 15, 0.001f));
}

static void test_vec2_scale(void) {
  Vec2 r = vec2_scale((Vec2){3, 4}, 2.0f);
  assert(is_near(r.x, 6, 0.001f));
  assert(is_near(r.y, 8, 0.001f));
}

static void test_vec2_length(void) {
  assert(is_near(vec2_length((Vec2){3, 4}), 5, 0.001f));
  assert(is_near(vec2_length((Vec2){0, 0}), 0, 0.001f));
}

static void test_vec2_normalize(void) {
  Vec2 r = vec2_normalize((Vec2){3, 0});
  assert(is_near(r.x, 1, 0.001f));
  assert(is_near(r.y, 0, 0.001f));
  assert(is_near(vec2_length(r), 1, 0.001f));

  r = vec2_normalize((Vec2){1, 1});
  f32 inv = 1.0f / sqrtf(2.0f);
  assert(is_near(r.x, inv, 0.001f));
  assert(is_near(r.y, inv, 0.001f));
}

static void test_vec2_distance(void) {
  assert(is_near(vec2_distance((Vec2){0, 0}, (Vec2){3, 4}), 5, 0.001f));
  assert(is_near(vec2_distance((Vec2){1, 2}, (Vec2){4, 6}), 5, 0.001f));
}

static void test_vec2_lerp(void) {
  Vec2 r = vec2_lerp((Vec2){0, 0}, (Vec2){10, 20}, 0.5f);
  assert(is_near(r.x, 5, 0.001f));
  assert(is_near(r.y, 10, 0.001f));

  r = vec2_lerp((Vec2){5, 10}, (Vec2){15, 30}, 0.0f);
  assert(is_near(r.x, 5, 0.001f));
  assert(is_near(r.y, 10, 0.001f));

  r = vec2_lerp((Vec2){5, 10}, (Vec2){15, 30}, 1.0f);
  assert(is_near(r.x, 15, 0.001f));
  assert(is_near(r.y, 30, 0.001f));
}

static void test_vec3_dot(void) {
  assert(is_near(vec3_dot((Vec3){1,0,0}, (Vec3){0,1,0}), 0, 0.001f));
  assert(is_near(vec3_dot((Vec3){1,2,3}, (Vec3){4,5,6}), 32, 0.001f));
}

static void test_vec3_cross(void) {
  Vec3 r = vec3_cross((Vec3){1,0,0}, (Vec3){0,1,0});
  assert(is_near(r.x, 0, 0.001f));
  assert(is_near(r.y, 0, 0.001f));
  assert(is_near(r.z, 1, 0.001f));
}

static void test_vec3_add(void) {
  Vec3 r = vec3_add((Vec3){1,2,3}, (Vec3){4,5,6});
  assert(is_near(r.x, 5, 0.001f));
  assert(is_near(r.y, 7, 0.001f));
  assert(is_near(r.z, 9, 0.001f));
}

static void test_vec3_sub(void) {
  Vec3 r = vec3_sub((Vec3){5,7,9}, (Vec3){4,5,6});
  assert(is_near(r.x, 1, 0.001f));
  assert(is_near(r.y, 2, 0.001f));
  assert(is_near(r.z, 3, 0.001f));
}

static void test_vec3_mul(void) {
  Vec3 r = vec3_mul((Vec3){2,3,4}, (Vec3){5,6,7});
  assert(is_near(r.x, 10, 0.001f));
  assert(is_near(r.y, 18, 0.001f));
  assert(is_near(r.z, 28, 0.001f));
}

static void test_vec3_scale(void) {
  Vec3 r = vec3_scale((Vec3){2,3,4}, 2.0f);
  assert(is_near(r.x, 4, 0.001f));
  assert(is_near(r.y, 6, 0.001f));
  assert(is_near(r.z, 8, 0.001f));
}

static void test_vec3_length(void) {
  assert(is_near(vec3_length((Vec3){1,2,2}), 3, 0.001f));
  assert(is_near(vec3_length((Vec3){0,0,0}), 0, 0.001f));
}

static void test_vec3_normalize(void) {
  Vec3 r = vec3_normalize((Vec3){2,0,0});
  assert(is_near(r.x, 1, 0.001f));
  assert(is_near(r.y, 0, 0.001f));
  assert(is_near(r.z, 0, 0.001f));

  r = vec3_normalize((Vec3){1,1,1});
  f32 inv3 = 1.0f / sqrtf(3.0f);
  assert(is_near(r.x, inv3, 0.001f));
  assert(is_near(r.y, inv3, 0.001f));
  assert(is_near(r.z, inv3, 0.001f));
}

static void test_vec3_distance(void) {
  assert(is_near(vec3_distance((Vec3){0,0,0}, (Vec3){1,2,2}), 3, 0.001f));
}

static void test_vec3_lerp(void) {
  Vec3 r = vec3_lerp((Vec3){0,0,0}, (Vec3){10,20,30}, 0.5f);
  assert(is_near(r.x, 5, 0.001f));
  assert(is_near(r.y, 10, 0.001f));
  assert(is_near(r.z, 15, 0.001f));
}

static void test_vec3_negate(void) {
  Vec3 r = vec3_negate((Vec3){1,-2,3});
  assert(is_near(r.x, -1, 0.001f));
  assert(is_near(r.y, 2, 0.001f));
  assert(is_near(r.z, -3, 0.001f));
}

static void test_vec4_dot(void) {
  assert(is_near(vec4_dot((Vec4){1,0,0,0}, (Vec4){0,1,0,0}), 0, 0.001f));
  assert(is_near(vec4_dot((Vec4){1,2,3,4}, (Vec4){5,6,7,8}), 70, 0.001f));
}

static void test_vec4_add(void) {
  Vec4 r = vec4_add((Vec4){1,2,3,4}, (Vec4){5,6,7,8});
  assert(is_near(r.x, 6, 0.001f));
  assert(is_near(r.y, 8, 0.001f));
  assert(is_near(r.z, 10, 0.001f));
  assert(is_near(r.w, 12, 0.001f));
}

static void test_vec4_sub(void) {
  Vec4 r = vec4_sub((Vec4){5,7,9,11}, (Vec4){4,5,6,7});
  assert(is_near(r.x, 1, 0.001f));
  assert(is_near(r.y, 2, 0.001f));
  assert(is_near(r.z, 3, 0.001f));
  assert(is_near(r.w, 4, 0.001f));
}

static void test_vec4_mul(void) {
  Vec4 r = vec4_mul((Vec4){2,3,4,5}, (Vec4){6,7,8,9});
  assert(is_near(r.x, 12, 0.001f));
  assert(is_near(r.y, 21, 0.001f));
  assert(is_near(r.z, 32, 0.001f));
  assert(is_near(r.w, 45, 0.001f));
}

static void test_vec4_scale(void) {
  Vec4 r = vec4_scale((Vec4){1,2,3,4}, 3.0f);
  assert(is_near(r.x, 3, 0.001f));
  assert(is_near(r.y, 6, 0.001f));
  assert(is_near(r.z, 9, 0.001f));
  assert(is_near(r.w, 12, 0.001f));
}

static void test_vec4_length(void) {
  assert(is_near(vec4_length((Vec4){1,2,2,0}), 3, 0.001f));
  assert(is_near(vec4_length((Vec4){0,0,0,0}), 0, 0.001f));
}

static void test_vec4_normalize(void) {
  Vec4 r = vec4_normalize((Vec4){4,0,0,0});
  assert(is_near(r.x, 1, 0.001f));
  assert(is_near(r.y, 0, 0.001f));
  assert(is_near(r.z, 0, 0.001f));
  assert(is_near(r.w, 0, 0.001f));
}

static void test_vec4_lerp(void) {
  Vec4 r = vec4_lerp((Vec4){0,0,0,0}, (Vec4){10,20,30,40}, 0.5f);
  assert(is_near(r.x, 5, 0.001f));
  assert(is_near(r.y, 10, 0.001f));
  assert(is_near(r.z, 15, 0.001f));
  assert(is_near(r.w, 20, 0.001f));
}

static void test_vec4_negate(void) {
  Vec4 r = vec4_negate((Vec4){1,-2,3,-4});
  assert(is_near(r.x, -1, 0.001f));
  assert(is_near(r.y, 2, 0.001f));
  assert(is_near(r.z, -3, 0.001f));
  assert(is_near(r.w, 4, 0.001f));
}

static void test_vec4_to_vec3(void) {
  Vec3 r = vec4_to_vec3((Vec4){3,6,9,3});
  assert(is_near(r.x, 1, 0.001f));
  assert(is_near(r.y, 2, 0.001f));
  assert(is_near(r.z, 3, 0.001f));
}

static void test_identity(void) {
  Mat4 id = MAT4_IDENTITY;
  for (i32 c = 0; c < 4; c++)
    for (i32 r = 0; r < 4; r++)
      assert(id.m[c][r] == (c == r ? 1.0f : 0.0f));
}

static void test_mul_identity(void) {
  Mat4 a = MAT4_IDENTITY;
  Mat4 b = MAT4_IDENTITY;
  Mat4 r = mat4_mul(a, b);
  assert(mat4_eq(r, a, 0.001f));
  r = mat4_mul(a, mat4_translation((Vec3){5, 10, 20}));
  assert(is_near(r.m[3][0], 5, 0.001f));
  assert(is_near(r.m[3][1], 10, 0.001f));
  assert(is_near(r.m[3][2], 20, 0.001f));
}

static void test_translate(void) {
  Mat4 m = mat4_translation((Vec3){10, 20, 30});
  assert(is_near(m.m[3][0], 10, 0.001f));
  assert(is_near(m.m[3][1], 20, 0.001f));
  assert(is_near(m.m[3][2], 30, 0.001f));
  assert(is_near(m.m[0][0], 1, 0.001f));
  assert(is_near(m.m[1][1], 1, 0.001f));
  assert(is_near(m.m[2][2], 1, 0.001f));
  assert(is_near(m.m[3][3], 1, 0.001f));
}

static void test_translate_compose(void) {
  Mat4 m = mat4_translate(MAT4_IDENTITY, (Vec3){5, 10, 15});
  assert(is_near(m.m[3][0], 5, 0.001f));
  assert(is_near(m.m[3][1], 10, 0.001f));
  assert(is_near(m.m[3][2], 15, 0.001f));
}

static void test_rotate_y(void) {
  Mat4 m = mat4_rotate_y(MAT4_IDENTITY, PI * 0.5f);
  assert(is_near(m.m[0][0], 0, 0.001f));
  assert(is_near(m.m[0][2], -1, 0.001f));
  assert(is_near(m.m[2][0], 1, 0.001f));
  assert(is_near(m.m[2][2], 0, 0.001f));
}

static void test_scale(void) {
  Mat4 m = mat4_scale(MAT4_IDENTITY, (Vec3){2, 3, 4});
  assert(is_near(m.m[0][0], 2, 0.001f));
  assert(is_near(m.m[1][1], 3, 0.001f));
  assert(is_near(m.m[2][2], 4, 0.001f));
  assert(is_near(m.m[3][3], 1, 0.001f));
}

static void test_transpose(void) {
  Mat4 m = MAT4_IDENTITY;
  m.m[0][1] = 7;
  m.m[2][3] = 13;
  Mat4 t = mat4_transpose(m);
  assert(is_near(t.m[1][0], 7, 0.001f));
  assert(is_near(t.m[3][2], 13, 0.001f));
  assert(mat4_eq(m, mat4_transpose(t), 0.001f));
}

static void test_inverse_translate(void) {
  Mat4 t = mat4_translation((Vec3){10, 20, 30});
  Mat4 inv = mat4_inverse(t);
  assert(is_near(inv.m[3][0], -10, 0.001f));
  assert(is_near(inv.m[3][1], -20, 0.001f));
  assert(is_near(inv.m[3][2], -30, 0.001f));
  assert(mat4_eq(mat4_mul(t, inv), MAT4_IDENTITY, 0.001f));
}

static void test_inverse_rotate(void) {
  Mat4 r = mat4_rotate_y(MAT4_IDENTITY, PI * 0.25f);
  Mat4 inv = mat4_inverse(r);
  assert(mat4_eq(mat4_mul(r, inv), MAT4_IDENTITY, 0.001f));
}

static void test_perspective(void) {
  Mat4 p = mat4_perspective(PI * 0.25f, 16.0f / 9.0f, 0.1f, 100.0f);
  assert(p.m[0][0] > 0);
  assert(p.m[1][1] < 0);
  assert(is_near(p.m[2][3], -1, 0.001f));
}

static void test_compose(void) {
  Mat4 m = mat4_translate(MAT4_IDENTITY, (Vec3){1, 2, 3});
  m = mat4_rotate_y(m, PI * 0.25f);
  m = mat4_scale(m, (Vec3){2, 2, 2});
  assert(!mat4_eq(m, MAT4_IDENTITY, 0.001f));
  assert(is_near(m.m[3][0], 5.656f, 0.01f));
  assert(is_near(m.m[3][1], 4, 0.01f));
  assert(is_near(m.m[3][2], 2.828f, 0.01f));
}

static void test_rotate_xyz(void) {
  Mat4 m = mat4_rotate(MAT4_IDENTITY, (Vec3){0.1f, 0.2f, 0.3f});
  assert(!mat4_eq(m, MAT4_IDENTITY, 0.001f));
  assert(is_near(m.m[0][0], 0.936f, 0.01f));
}

int main(void) {
  test_vec2_dot();
  test_vec2_cross();
  test_vec2_add();
  test_vec2_sub();
  test_vec2_mul();
  test_vec2_scale();
  test_vec2_length();
  test_vec2_normalize();
  test_vec2_distance();
  test_vec2_lerp();
  test_vec3_dot();
  test_vec3_cross();
  test_vec3_add();
  test_vec3_sub();
  test_vec3_mul();
  test_vec3_scale();
  test_vec3_length();
  test_vec3_normalize();
  test_vec3_distance();
  test_vec3_lerp();
  test_vec3_negate();
  test_vec4_dot();
  test_vec4_add();
  test_vec4_sub();
  test_vec4_mul();
  test_vec4_scale();
  test_vec4_length();
  test_vec4_normalize();
  test_vec4_lerp();
  test_vec4_negate();
  test_vec4_to_vec3();
  test_identity();
  test_mul_identity();
  test_translate();
  test_translate_compose();
  test_rotate_y();
  test_scale();
  test_transpose();
  test_inverse_translate();
  test_inverse_rotate();
  test_perspective();
  test_compose();
  test_rotate_xyz();
  printf("All math tests passed!\n");
  return 0;
}
