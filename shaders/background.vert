#version 450

layout(location = 0) out vec2 uv;

void main() {
    float x = (gl_VertexIndex == 0) ? -1.0
            : (gl_VertexIndex == 1) ?  3.0
                                    : -1.0;
    float y = (gl_VertexIndex == 0) ? -1.0
            : (gl_VertexIndex == 1) ? -1.0
                                    :  3.0;
    gl_Position = vec4(x, y, 0.0, 1.0);
    uv = vec2(x, y) * 0.5 + 0.5;
}
