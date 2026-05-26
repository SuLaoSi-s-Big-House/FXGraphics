#version 430 core

in vec4 aPosInView;
in vec4 aNormInView;
in vec4 aColor;
out vec4 FragColor;

void main()
{
    FragColor = aColor;
}
