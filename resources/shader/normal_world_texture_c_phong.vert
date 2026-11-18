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
    mat4 modelView = vMatrix * model;
    aPosInView = modelView * vec4(aPos, 1);
    gl_Position = pMatrix * aPosInView;

    mat3 normalMatrix = transpose(inverse(mat3(modelView)));
    aNormInView = vec4(normalize(normalMatrix * aNormal), 0);

    aColor = EntityProfile[aRank.y].color;

    vec4 uvScale = EntityProfile[aRank.y].uvScale1;
    aColorTexCoord = vec3(aUv.xy * uvScale.xy, uvScale.z);
}
