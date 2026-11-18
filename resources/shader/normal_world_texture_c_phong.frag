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

    vec3 normal = normalize(aNormInView.xyz);
    vec3 lightPos = vec3(50, 100, 100);
    vec3 ambient = vec3(0.2, 0.2, 0.2);
    vec3 lightDir = normalize(lightPos - aPosInView.xyz);
    vec3 viewDir = vec3(0, 0, 1);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 diffuse = max(dot(lightDir, normal), 0) * color.rgb * 0.6;
    vec3 specular = pow(max(dot(normal, halfwayDir), 0), 8) * color.rgb * 0.15;

    FragColor = vec4(ambient + diffuse + specular, color.a);
}
