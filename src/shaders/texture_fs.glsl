#version 450 core

in vec2 texture_coord;
layout(binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;


void main()
{
    outColor = texture(tex, texture_coord);
}
