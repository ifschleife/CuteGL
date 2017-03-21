#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal_in;

layout(location = 0) out vec3 normal;

layout(std140, binding = 0) uniform UniformsForVS
{
    mat4 mvp;
};


void main()
{
    normal = normal_in;
    gl_Position = mvp * vec4(position, 1.0);
}
