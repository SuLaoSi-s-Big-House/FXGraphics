#version 430 core

uniform globalConfig {
    mat4 mvpMatrix;
    mat4 mvMatrix;
    vec4 color;
    vec4 lightPos;
};

in vec4 vs_posInView;
in vec4 vs_normalInView;

out vec4 fr_color;

void main()
{
    vec3 ambient = 0.3 * color.xyz;
    vec3 normal = normalize(vs_normalInView.xyz);
    vec3 lightDir = normalize((lightPos - vs_posInView).xyz);
    vec3 diffuse = 0.7 * color.xyz * max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + vec3(0.0, 0.0, 1.0));
    vec3 specular = 0.1 * color.xyz * pow(max(dot(normal, halfwayDir), 0.0), 8);
    fr_color = vec4(ambient + diffuse + specular, color.a);
}