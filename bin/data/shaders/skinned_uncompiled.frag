#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 2) uniform sampler2D diffuseSampler;
layout (binding = 3) uniform sampler2D specularSampler;
layout (binding = 4) uniform sampler2D normalSampler;

layout (binding = 5) uniform UBO
{
	float materialSpecStrength;
	float materialShininess;
	float hasSpecularMap;
	float hasNormalMap;
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
	
	if(ubo.hasSpecularMap == 1.0f)
		outMaterial = texture(specularSampler, texCoord);
	else
		outMaterial = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		
	outMaterial.g = ubo.materialShininess;
	outMaterial.b = ubo.materialSpecStrength;
	
	// If there is a normal map available overwrite normals
	if(ubo.hasNormalMap == 1.0f)
	{
		vec3 tempNormal = texture(normalSampler, texCoord).rgb;
		tempNormal = normalize(tempNormal * 2.0f - 1.0f);
		tempNormal = normalize(tangentSpace * tempNormal);
		outNormal = vec4(tempNormal, 1.0f);
	}
}