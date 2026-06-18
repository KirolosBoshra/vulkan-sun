#ifndef MODEL_H
#define MODEL_H

#include "render/resource.h"
#include "utils/color.h"
#include "utils/math.h"

typedef struct {
  MeshHandle mesh;
  Mat4 transform;
  Color color;
  // Texture texture;
} Model;

typedef struct {
  Mat4 model;
  Color color;
} ModelPush;

static inline Model model_create(MeshHandle mesh) {
  Model model;
  model.mesh = mesh;
  model.transform = MAT4_IDENTITY;
  model.color = COLOR_WHITE;
  return model;
}

// we don't need model destroy because the mesh will be destroyed separately and
// the model doesn't own any resources
// void model_destroy(Model *model) {}

#endif // !MODEL_H
