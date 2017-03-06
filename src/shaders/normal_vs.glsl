#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal_in;
layout(location = 2) in vec2 texcoord_in;

out vec3 normal;
out vec2 texture_coord;

layout(std140, binding = 0) uniform UniformsForVS
{
    mat4 matrix;
};


void main()
{
    normal = normal_in;
    texture_coord = texcoord_in;
    gl_Position = matrix * vec4(position, 1.0);
}
