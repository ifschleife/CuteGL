#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal_in;

out vec3 normal;

layout(std140, binding = 0) uniform UniformsForVS
{
    mat4 matrix;
};


void main()
{
    normal = normal_in;
    gl_Position = matrix * vec4(position, 1.0);
}
