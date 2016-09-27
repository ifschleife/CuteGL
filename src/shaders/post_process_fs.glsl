#version 450 core

in vec3 normal;

layout(location = 0) out vec4 outColor;
uniform sampler2D tex;

void main()
{
	vec4 col = texelFetch(tex, ivec2(gl_FragCoord.xy), 0).rgba;
	outColor = vec4(vec3(1.0) - col.rgb, 1.0);
};
