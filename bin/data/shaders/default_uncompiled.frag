#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;

layout (binding = 4) uniform UBO
{
	vec4 ambientColor;
	vec4 diffuseColor;
	vec3 lightDirection;
	int imageIndex;
} ubo;

layout (location = 0) in vec2 texCoord;

layout (location = 0) out vec4 outColor;

void main()
{
	vec3 fragPos = texture(samplerPosition, texCoord).rgb;
	vec3 normal = texture(samplerNormal, texCoord).rgb;
	vec4 albedo = texture(samplerAlbedo, texCoord);
	
	if(ubo.imageIndex == 4)
	{		
		vec3 fragColor = albedo.rgb * ubo.ambientColor.rgb;
		vec3 lightDir = -ubo.lightDirection;
		vec3 diff = max(dot(normal, lightDir), 0.0f) * albedo.rgb * ubo.diffuseColor.rgb;
		fragColor += diff;
		outColor = vec4(fragColor, 1.0f);
	}
	else if(ubo.imageIndex == 3)
		outColor = albedo;
	else if(ubo.imageIndex == 2)
		outColor = vec4(normal, 1.0f);
	else
		outColor = vec4(fragPos, 1.0f);
}