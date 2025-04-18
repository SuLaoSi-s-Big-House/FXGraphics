#version 430 core

uniform globalConfig {
    mat4 mvpMatrix;
    mat4 mvMatrix;
    vec4 color;
    vec4 lightPos;
};

in vec4 vs_posInView;
in vec4 vs_normalInView;

out vec4 fr_color;

void main()
{
    fr_color = color;
}