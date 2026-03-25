#version 450

layout(location = 0) in vec2 aVertex;
layout(location = 1) in vec3 aColor;

layout(push_constant) uniform pc {
    mat4 projection;
    mat4 model;
};
//vec2 positions[] = vec2[](
//    vec2(0.0, -0.5),
//    vec2(0.5, 0.5),
//    vec2(-0.5, 0.5)
//);
//
//vec3 colors[] = vec3[](
//    vec3(1.0, 0.0, 0.0),
//    vec3(0.0, 1.0, 0.0),
//    vec3(0.0, 0.0, 1.0)
//);

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = projection * model * vec4(aVertex, 0.0, 1.0);

    fragColor = aColor;
}