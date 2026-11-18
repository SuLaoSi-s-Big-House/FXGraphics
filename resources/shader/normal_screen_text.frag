#version 430 core

#include "normal_uniform.h"

layout(binding = 6) uniform sampler2DArray TextTexture;

in vec3 aTexCoord;

in vec4 aColor;
out vec4 FragColor;

void main()
{
    float alpha = texture(TextTexture, aTexCoord).r * aColor.a;

    if (renderPass == 0)
    {
        if (alpha < 1.0)
        {
            discard;
        }
    }
    else
    {
        if (alpha <= 0.0 || alpha >= 1.0)
        {
            discard;
        }
    }

    FragColor = vec4(aColor.rgb, alpha);
}
