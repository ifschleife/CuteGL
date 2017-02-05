#version 450 core

layout(location = 0) in vec3 position;

out vec3 normal;

layout(std140, binding = 0) uniform UniformsForVS
{
    mat4 matrix;
};


void main()
{
    normal = normalize(position);
    gl_Position = matrix * vec4(position, 1.0);
}
