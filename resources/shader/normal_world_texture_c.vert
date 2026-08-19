#version 430 core

#include "normal_input.h"
#include "normal_uniform.h"
#include "normal_texture_profile.h"

out vec4 aPosInView;
out vec4 aNormInView;
out vec4 aColor;
out vec3 aColorTexCoord;

void main()
{
    mat4 model = EntityProfile[aRank.y].model;
    aPosInView = vMatrix * model * vec4(aPos, 1);
    gl_Position = pMatrix * aPosInView;
    aNormInView = normalize(vMatrix * model * vec4(aNormal, 0));

    aColor = EntityProfile[aRank.y].color;

    // image可能未占满slice，uv需要按实际范围缩放，z为slice下标
    vec4 uvScale = EntityProfile[aRank.y].uvScale1;
    aColorTexCoord = vec3(aUv.xy * uvScale.xy, uvScale.z);
}
