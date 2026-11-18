#version 430 core

#include "normal_uniform.h"
#include "normal_texture_input.h"

in vec4 aPosInView;
in vec4 aNormInView;
in vec4 aColor;
in vec3 aColorTexCoord;
out vec4 FragColor;

void main()
{
    vec4 color = texture(BaseColorTexture, aColorTexCoord) * aColor;

    if (renderPass == 0)
    {
        if (color.a < 1.0)
        {
            discard;
        }
    }
    else
    {
        if (color.a <= 0.0 || color.a >= 1.0)
        {
            discard;
        }
    }

    FragColor = color;
}
