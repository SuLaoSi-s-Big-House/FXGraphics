#version 430 core

layout(binding = 6) uniform sampler2DArray TextTexture;

in vec3 aTexCoord;

in vec4 aColor;
out vec4 FragColor;

void main()
{
    float alpha = texture(TextTexture, aTexCoord).r;
    FragColor = vec4(aColor.rgb, aColor.a * alpha);
}
