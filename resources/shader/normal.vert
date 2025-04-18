#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform globalConfig {
    mat4 mvpMatrix;
    mat4 mvMatrix;
    vec4 color;
    vec4 lightPos;
};

out vec4 vs_posInView;
out vec4 vs_normalInView;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);
    vs_posInView = mvMatrix * vec4(position, 1.0);
    vs_normalInView = transpose(inverse(mvMatrix)) * vec4(normal, 0.0);
}
