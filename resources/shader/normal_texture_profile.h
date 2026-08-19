struct NormalTextureProfileData {
    mat4 model;
    vec4 color;
    vec4 uvScale1;
    vec4 uvScale2;
    vec4 uvScale3;
    vec4 custom1;
    vec4 custom2;
};

layout(std430, binding = 1) readonly buffer ProfileData {
    NormalTextureProfileData EntityProfile[];
};
