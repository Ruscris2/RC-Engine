#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define CASCADE_COUNT 3

layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;
layout (binding = 4) uniform sampler2D samplerSpecular;
layout (binding = 5) uniform sampler2D samplerDepth;

layout (binding = 6) uniform UBO
{
	mat4 lightViewMatrix[CASCADE_COUNT];
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
	vec3 lightDirection;
	int imageIndex;
	vec3 cameraPosition;
	float padding;
} ubo;

layout (binding = 7) uniform sampler2DArray samplerShadowMap;

layout (location = 0) in vec2 texCoord;

layout (location = 0) out vec4 outColor;

float TextureProjection(vec4 projCoord, int layer)
{	
	float shadow = 1.0f;
	
	if(clamp(projCoord.x, 0.0f, 1.0f) == projCoord.x && clamp(projCoord.y, 0.0f, 1.0f) == projCoord.y)
	{
		if(projCoord.z > -1.0f && projCoord.z < 1.0f)
		{
			float distance = texture(samplerShadowMap, vec3(projCoord.xy, layer)).r;
			if(projCoord.w > 0.0f && distance < projCoord.z)
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
		float shadow = 1.0f;
		vec2 projectCoords[CASCADE_COUNT];
		
		// Find the shadow cascade for this fragment
		for(int i = 0; i < CASCADE_COUNT; i++)
		{
			vec4 shadowClip = ubo.lightViewMatrix[i] * vec4(fragPos, 1.0f);
			projectCoords[i].x = shadowClip.x / shadowClip.w / 2.0f + 0.5f;
			projectCoords[i].y = shadowClip.y / shadowClip.w / 2.0f + 0.5f;
			
			if(clamp(projectCoords[i].x, 0.0f, 1.0f) == projectCoords[i].x && clamp(projectCoords[i].y, 0.0f, 1.0f) == projectCoords[i].y)
			{
				float mapDepth = texture(samplerShadowMap, vec3(projectCoords[i], i)).r;
				float lightDepth = shadowClip.z / shadowClip.w;
				if(lightDepth < -1.0f || lightDepth > 1.0f)
					continue;
				
				if(lightDepth > mapDepth)
				{
					shadow = 0.25f;
					break;
				}
			}
		}
		
		vec3 viewDir = normalize(ubo.cameraPosition - fragPos);
		vec4 fragColor = ubo.ambientColor;
		vec4 spec = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		vec3 lightDir = -ubo.lightDirection;
		float lightIntensity = dot(normal, lightDir);
		
		if(lightIntensity > 0.0f)
		{
			vec4 diff = clamp(lightIntensity * ubo.diffuseColor, 0.0f, 1.0f) * shadow;
			
			vec3 halfVec = normalize(lightDir + viewDir);
			spec = ubo.specularColor * pow(max(dot(normal, halfVec), 0.0), specular.g) * specular.b * specular.r;
			if(specular.g  == 0.0f)
				spec = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			
			fragColor += diff;
		}
		
		fragColor = clamp(fragColor, 0.0f, 1.0f);
		outColor = (fragColor * albedo) + (spec * shadow);
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