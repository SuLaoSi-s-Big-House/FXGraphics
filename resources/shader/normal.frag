#version 430 core

uniform globalConfig {
    mat4 vpMatrix;
    vec4 color;
};

in vec4 vs_normal;

out vec4 fr_color;

void main()
{
    fr_color = color;
}