#version 450 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texcoord_in;

layout(location = 0) out vec2 texture_coord;

layout(std140, binding = 0) uniform UniformsForVS
{
    mat4 matrix;
};


void main()
{
    texture_coord = texcoord_in;
    gl_Position = matrix * vec4(position, 1.0);
}
