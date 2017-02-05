#version 450 core

in vec2 texture_coord;
layout(binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;


void main()
{
    vec3 coord = texture(tex, texture_coord).rgb;
    float alpha = 1.0;
    if (coord.x == 1.0 && coord.y == 1.0 && coord.z == 1.0)
        alpha = 0.4;
    outColor = vec4(coord, alpha);
}
