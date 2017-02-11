#version 450 core

in vec3 normal;

layout(location = 0) out vec4 outColor;
uniform sampler2D tex;

void main()
{
    outColor = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);
}
