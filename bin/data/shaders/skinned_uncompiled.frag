#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D diffuseSampler;
layout (binding = 2) uniform sampler2D specularSampler;

layout (binding = 3) uniform UBO
{
	float materialSpecStrength;
	float materialShininess;
	vec2 padding;
} ubo;

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normals;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outSpecular;

void main()
{
	outPosition = vec4(worldPos, 1.0f);
	outNormal = vec4(normals, 1.0f);
	outAlbedo = texture(diffuseSampler, texCoord);
	outSpecular = texture(specularSampler, texCoord);
	
	outPosition.a = gl_FragCoord.z;
	outSpecular.g = ubo.materialShininess;
	outSpecular.b = ubo.materialSpecStrength;
}