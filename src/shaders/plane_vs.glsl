#version 430 core

layout(location = 0) in vec3 position;
layout(std140, binding = 0) uniform UniformsForVS
{
	mat4 matrix;
};

out vec2 texture_coord;

void main()
{
    texture_coord = position.xz;
	gl_Position = matrix * vec4(position, 1.0);
}
