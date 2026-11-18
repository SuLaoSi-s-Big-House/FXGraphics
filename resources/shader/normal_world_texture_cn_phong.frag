#version 430 core

#include "normal_uniform.h"
#include "normal_texture_input.h"

in vec4 aPosInView;
in vec4 aNormInView;
in vec4 aColor;
in vec3 aColorTexCoord;
in vec3 aNormalTexCoord;
out vec4 FragColor;

void main()
{
    vec4 color = texture(BaseColorTexture, aColorTexCoord) * aColor;

    vec3 normal = aNormInView.xyz;
    float normalLen2 = dot(normal, normal);

    if (normalLen2 < 1e-12 || any(isnan(normal)) || any(isinf(normal)))
    {
        normal = vec3(0, 0, 1);
    }
    else
    {
        normal *= inversesqrt(normalLen2);
    }

    // 用view空间位置和normal map的UV导数构造TBN
    vec3 dp1 = dFdx(aPosInView.xyz);
    vec3 dp2 = dFdy(aPosInView.xyz);
    vec2 duv1 = dFdx(aNormalTexCoord.xy);
    vec2 duv2 = dFdy(aNormalTexCoord.xy);

    vec3 dp2perp = cross(dp2, normal);
    vec3 dp1perp = cross(normal, dp1);
    vec3 tangent = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 bitangent = dp2perp * duv1.y + dp1perp * duv2.y;

    float maxLen2 = max(dot(tangent, tangent), dot(bitangent, bitangent));

    if (maxLen2 < 1e-12 || any(isnan(tangent)) || any(isinf(tangent)) ||
        any(isnan(bitangent)) || any(isinf(bitangent)))
    {
        vec3 up = abs(normal.y) < 0.999 ? vec3(0, 1, 0) : vec3(1, 0, 0);
        tangent = normalize(cross(up, normal));
        bitangent = cross(normal, tangent);
    }
    else
    {
        tangent = tangent - normal * dot(normal, tangent);
        float tangentLen2 = dot(tangent, tangent);

        if (tangentLen2 < 1e-12)
        {
            vec3 up = abs(normal.y) < 0.999 ? vec3(0, 1, 0) : vec3(1, 0, 0);
            tangent = normalize(cross(up, normal));
            bitangent = cross(normal, tangent);
        }
        else
        {
            tangent = normalize(tangent);

            // 根据原始bitangent确定handedness，兼容镜像UV
            float signB = dot(bitangent, cross(normal, tangent)) < 0.0 ? -1.0 : 1.0;
            bitangent = cross(normal, tangent) * signB;
        }
    }

    vec3 tangentNormal = texture(NormalTexture, aNormalTexCoord).xyz * 2.0 - 1.0;
    float tangentNormalLen2 = dot(tangentNormal, tangentNormal);

    if (tangentNormalLen2 < 1e-12 || any(isnan(tangentNormal)) || any(isinf(tangentNormal)))
    {
        tangentNormal = vec3(0, 0, 1);
    }
    else
    {
        tangentNormal *= inversesqrt(tangentNormalLen2);
    }

    normal = normalize(tangent * tangentNormal.x + bitangent * tangentNormal.y + normal * tangentNormal.z);

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

    vec3 lightPos = vec3(50, 100, 100);
    vec3 ambient = vec3(0.2, 0.2, 0.2);
    vec3 lightDir = normalize(lightPos - aPosInView.xyz);
    vec3 viewDir = vec3(0, 0, 1);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 diffuse = max(dot(lightDir, normal), 0) * color.rgb * 0.6;
    vec3 specular = pow(max(dot(normal, halfwayDir), 0), 8) * color.rgb * 0.15;

    FragColor = vec4(ambient + diffuse + specular, color.a);
}
