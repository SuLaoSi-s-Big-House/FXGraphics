#version 430 core

#include "normal_uniform.h"

in vec4 aPosInView;
in vec4 aNormInView;
in vec4 aColor;
out vec4 FragColor;

void main()
{
    vec3 lightPos = vec3(50, 100, 100);
    vec3 ambient = vec3(0.2, 0.2, 0.2);
    vec4 normal = normalize(aNormInView);
    vec4 lightDir = normalize(vec4(lightPos, 1) - aPosInView);
    vec3 diffuse = max(dot(lightDir, normal), 0) * aColor.rgb * 0.6;
    vec4 viewDir = vec4(0, 0, 1, 0);
    vec4 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(normal, halfwayDir), 0), 8) * aColor.rgb * 0.15;
    FragColor = vec4(ambient + diffuse + specular, aColor.a);
}
