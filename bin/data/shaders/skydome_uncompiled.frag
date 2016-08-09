#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform UBO
{
	vec4 skyColor;
	vec4 atmosphereColor;
	vec4 groundColor;
	float atmosphereHeight;
	vec3 padding;
} ubo;

layout (location = 0) in float height;

layout (location = 0) out vec4 outColor;

void main()
{
	if(height < 0.0f)
		outColor = ubo.groundColor;
	else
		outColor = mix(ubo.atmosphereColor, ubo.skyColor, height + ubo.atmosphereHeight);
		
	gl_FragDepth = 0.999999f;
}