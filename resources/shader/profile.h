struct NormalProfileData {
    mat4 model;
    vec4 color;
};

layout(std430, binding = 1) readonly buffer ProfileData {
    NormalProfileData EntityProfile[];
};
