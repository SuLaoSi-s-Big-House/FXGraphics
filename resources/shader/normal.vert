#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform globalConfig {
    mat4 vpMatrix;
    vec4 color;
};

out vec4 vs_normal;

void main()
{
    gl_Position = vpMatrix * vec4(0.03 * position, 1.0);
    vs_normal = vpMatrix * vec4(normal, 0.0);
}
