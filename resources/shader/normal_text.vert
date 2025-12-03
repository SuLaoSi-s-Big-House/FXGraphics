#version 430 core

// #include "input.h"

// #include "profile.h"

out vec3 uv;
out vec4 aColor;

void main()
{
    mat4 model = EntityProfile[aRank.y].model;
    vec4 pos = model * vec4(aPos.xy, 0.0, 1.0);
    gl_Position = vec4((pos.x / 400) - 1, -(pos.y / 300 - 1), -0.999, 1.0);
    uv = aUv;
    aColor = EntityProfile[aRank.y].color;
}
