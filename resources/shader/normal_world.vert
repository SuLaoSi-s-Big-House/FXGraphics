#version 430 core

#include "normal_input.h"
#include "normal_uniform.h"
#include "normal_profile.h"

out vec4 aColor;

void main()
{
    mat4 model = EntityProfile[aRank.y].model;
    gl_Position = vpMatrix * model * vec4(aPos * 0.1, 1.0);

    aColor = EntityProfile[aRank.y].color;
}
