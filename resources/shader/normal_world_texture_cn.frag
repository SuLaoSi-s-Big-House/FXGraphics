#version 430 core

#include "normal_texture_input.h"

in vec4 aPosInView;
in vec4 aNormInView;
in vec4 aColor;
in vec3 aColorTexCoord;
in vec3 aNormalTexCoord;
out vec4 FragColor;

void main()
{
    vec4 texColor = texture(ColorTexture, aColorTexCoord);
    // TODO 光照实现后，使用texture(NormalTexture, aNormalTexCoord)计算法向
    FragColor = texColor * aColor;
}
