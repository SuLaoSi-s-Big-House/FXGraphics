#version 430 core

#include "normal_input.h"
#include "normal_uniform.h"
#include "normal_profile.h"

out vec3 aTexCoord;
out vec4 aColor;

void main()
{
    mat4 model = EntityProfile[aRank.y].matrix;
    uint handle = EntityProfile[aRank.y].materialHandle.x;

    vec2 pos = 2 * (model * vec4(aPos, 1.0)).xy / viewport;
    gl_Position = vec4(pos.x - 1, 1 - pos.y, EntityMaterial[handle].custom1.x, 1.0);

    aTexCoord = aUv;
    
    aColor = EntityMaterial[handle].rgba;
}
