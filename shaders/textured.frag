#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main() {
    outColor = texture(tex, fragUV) * fragColor;
}
