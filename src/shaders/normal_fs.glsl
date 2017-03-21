#version 450 core

layout(location = 0) in vec3 normal;

layout(location = 0) out vec4 outColor;


void main()
{
    outColor = vec4(abs(normal.rgb), 1.0);
}
