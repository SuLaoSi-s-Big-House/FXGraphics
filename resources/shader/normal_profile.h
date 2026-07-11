struct NormalProfileData {
    mat4 matrix;
    uvec4 materialHandle;
};

layout(std430, binding = 1) readonly buffer ProfileData {
    NormalProfileData EntityProfile[];
};

struct NormalMaterialData {
    vec4 rgba;
    vec4 pbr;
    vec4 custom1;
    vec4 custom2;
};

layout(std430, binding = 2) readonly buffer MaterialData {
    NormalMaterialData EntityMaterial[];
};
