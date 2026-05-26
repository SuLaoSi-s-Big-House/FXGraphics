#version 430 core

#include "normal_input.h"
#include "normal_uniform.h"
#include "normal_profile.h"

out vec4 aPosInView;
out vec4 aNormInView;
out vec4 aColor;

void main()
{
    mat4 model = EntityProfile[aRank.y].model;
    aPosInView = vMatrix * model * vec4(aPos, 1);
    gl_Position = pMatrix * aPosInView;
    aNormInView = normalize(vMatrix * model * vec4(aNormal, 0));

    aColor = EntityProfile[aRank.y].color;
}
