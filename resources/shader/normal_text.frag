#version 430 core

in vec3 uv;
in vec4 aColor;
out vec4 FragColor;

uniform sampler2DArray diffuseMap;

void main()
{
    vec4 diffuseColor = texture(diffuseMap, uv);
    FragColor = aColor * diffuseColor.x;
}
