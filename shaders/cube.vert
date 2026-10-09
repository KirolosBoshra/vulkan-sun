#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUV;

layout(binding = 0) uniform UBO {
    mat4 vp;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
    vec4 color;
} pc;

void main() {
    gl_Position = ubo.vp * pc.model * vec4(position, 1.0);
    fragColor = color * pc.color;
    fragUV = uv;
}
