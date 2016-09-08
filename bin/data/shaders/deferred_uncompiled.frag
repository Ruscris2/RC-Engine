#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D diffuseSampler;
layout (binding = 2) uniform sampler2D materialSampler;
layout (binding = 3) uniform sampler2D normalSampler;

layout (binding = 4) uniform UBO
{
	float hasNormalMap;
	float metallicOffset;
	float roughnessOffset;
	float padding;
} ubo;

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normals;
layout (location = 3) in mat3 tangentSpace;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMaterial;

void main()
{
	outPosition = vec4(worldPos, 1.0f);
	outNormal = vec4(normals, 1.0f);
	outAlbedo = texture(diffuseSampler, texCoord);
	
	outMaterial = texture(materialSampler, texCoord);
	outMaterial.r = clamp(outMaterial.r + ubo.metallicOffset, 0.0f, 1.0f);
	outMaterial.g = clamp(outMaterial.g + ubo.roughnessOffset, 0.0f, 1.0f);
	
	// If there is a normal map available overwrite normals
	if(ubo.hasNormalMap == 1.0f)
	{
		vec3 tempNormal = texture(normalSampler, texCoord).rgb;
		tempNormal = normalize(tempNormal * 2.0f - 1.0f);
		tempNormal = normalize(tangentSpace * tempNormal);
		outNormal = vec4(tempNormal, 1.0f);
	}
}