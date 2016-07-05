#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D colorSampler;

layout (binding = 2) uniform UBO
{
	vec4 ambientColor;
	vec4 diffuseColor;
	vec3 lightDirection;
	float padding;
} ubo;

layout (location = 0) in vec2 texCoord;
layout (location = 1) in vec3 normals;

layout (location = 0) out vec4 outColor;

void main()
{
	vec4 textureColor = texture(colorSampler, texCoord, 0.0f);
	
	vec3 lightDir = -ubo.lightDirection;
	float lightIntensity = max(dot(normals, lightDir), 0.0f);
	
	outColor = ubo.ambientColor + ubo.diffuseColor * lightIntensity;
	outColor = outColor * textureColor;
}