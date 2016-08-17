#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;
layout (binding = 4) uniform sampler2D samplerSpecular;
layout (binding = 5) uniform sampler2D samplerDepth;

layout (binding = 6) uniform UBO
{
	mat4 lightViewMatrix;
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
	vec3 lightDirection;
	int imageIndex;
	vec3 cameraPosition;
	float padding;
} ubo;

layout (binding = 7) uniform sampler2D samplerShadowMap;

layout (location = 0) in vec2 texCoord;

layout (location = 0) out vec4 outColor;

float textureProj(vec4 coord)
{
	float shadow = 1.0f;
	vec4 shadowCoord = coord / coord.w;
	shadowCoord.st = shadowCoord.st * 0.5f + 0.5f;
	
	if(shadowCoord.z > -1.0f && shadowCoord.z < 1.0f)
	{
		float distance = texture(samplerShadowMap, shadowCoord.st).r;
		if(shadowCoord.w > 0.0f && distance < shadowCoord.z)
		{			
			shadow = 0.25f;
		}
	}
	
	return shadow;
}

void main()
{
	vec3 fragPos = texture(samplerPosition, texCoord).rgb;
	vec3 normal = texture(samplerNormal, texCoord).rgb;
	vec4 albedo = texture(samplerAlbedo, texCoord);
	vec4 specular = texture(samplerSpecular, texCoord);
	
	gl_FragDepth = texture(samplerDepth, texCoord).b;
	
	if(ubo.imageIndex == 5)
	{	
		vec3 viewDir = normalize(ubo.cameraPosition - fragPos);
		vec3 fragColor = clamp(albedo.rgb * ubo.ambientColor.rgb, 0.0, 1.0);
		vec3 lightDir = -ubo.lightDirection;
		
		vec3 diff = max(dot(normal, lightDir), 0.0f) * albedo.rgb * ubo.diffuseColor.rgb;
		
		vec3 halfVec = normalize(lightDir + viewDir);
		vec3 spec = ubo.specularColor.rgb * pow(max(dot(normal, halfVec), 0.0), specular.g) * specular.b * specular.r;
		if(specular.g  == 0.0f)
			spec = vec3(0.0f, 0.0f, 0.0f);
		
		fragColor += clamp(diff + spec, 0.0, 1.0);
		
		vec4 shadowClip = ubo.lightViewMatrix * vec4(fragPos, 1.0f);
		float shadowFactor = textureProj(shadowClip);
		
		outColor = vec4(fragColor, 1.0f) * shadowFactor;
	}
	else if(ubo.imageIndex == 4)
	{
		outColor = specular;
	}
	else if(ubo.imageIndex == 3)
		outColor = albedo;
	else if(ubo.imageIndex == 2)
		outColor = vec4(normal, 1.0f);
	else
		outColor = vec4(fragPos, 1.0f);
}