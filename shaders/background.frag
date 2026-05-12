#version 450

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    float time;
} pc;

void main() {
    vec3 col = 0.5 + 0.5 * cos(uv.xyx + vec3(0.0, 2.0, 4.0) + pc.time);
    outColor = vec4(col, 1.0);
}
