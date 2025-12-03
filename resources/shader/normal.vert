#version 430 core

// #include "input.h"

// #include "profile.h"

out vec4 aColor;

void main()
{
    mat4 model = EntityProfile[aRank.y].model;
    aColor = EntityProfile[aRank.y].color;
    gl_Position = model * vec4(aPos.xyz * 0.1, 1.0);
}
