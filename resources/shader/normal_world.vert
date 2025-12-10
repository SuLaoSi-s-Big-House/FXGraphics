#version 430 core

// #include "normal_input.h"

// #include "normal_profile.h"

out vec4 aColor;

void main()
{
    mat4 model = EntityProfile[aRank.y].model;
    aColor = EntityProfile[aRank.y].color;
    gl_Position = model * vec4(aPos, 1.0);
}
