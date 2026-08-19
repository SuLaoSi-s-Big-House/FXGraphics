#version 430 core

#include "normal_texture_input.h"

in vec4 aPosInView;
in vec4 aNormInView;
in vec4 aColor;
in vec3 aColorTexCoord;
out vec4 FragColor;

void main()
{
    vec4 texColor = texture(ColorTexture, aColorTexCoord);
    FragColor = texColor * aColor;
}
