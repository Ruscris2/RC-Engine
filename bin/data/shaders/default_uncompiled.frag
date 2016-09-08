#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//========================================== DEFINES ================================================
#define CASCADE_COUNT 3
#define MAX_LIGHTS 32

//========================================== UNIFORMS ===============================================
layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;
layout (binding = 4) uniform sampler2D samplerMaterial;
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
	float shadowStrength;
} ubo;

layout (binding = 7) uniform sampler2DArray samplerShadowMap;

struct PointLight
{
	vec4 lightColor;
	vec3 lightPosition;
	float radius;
};

layout (binding = 8) uniform LightBuffer
{
	PointLight lights[MAX_LIGHTS];
	int lightCount;
	vec3 padding;
} lightBuffer;

layout (binding = 9) uniform samplerCube samplerCubeMap;

//====================================== PIXEL SHADER I/O ===========================================
layout (location = 0) in vec2 texCoord;

layout (location = 0) out vec4 outColor;

//============================== PHYSICALLY BASED RENDERING FUNCTIONS ===============================
vec3 CalculateFresnelReflectance(vec3 viewDir, vec3 halfVec, vec3 specular)
{
	return specular + (1.0f - specular) * pow(1.0f - (dot(halfVec, viewDir)), 5.0f);
}

float CalculateSmithGGXGeometryTerm(float roughness, float nDotL, float nDotV)
{
	float roughnessActual = roughness * roughness;
	float viewGeoTerm = nDotV + sqrt( (nDotV - nDotV * roughnessActual) * nDotV + roughnessActual );
	float lightGeoTerm = nDotL + sqrt( (nDotL - nDotL * roughnessActual) * nDotL + roughnessActual );
	
	return 1.0f / (viewGeoTerm * lightGeoTerm);
}

float CalculateNormalDistributionTrowReitz(float roughness, vec3 surfaceNormal, vec3 microfacetNormal)
{
	float PI = 3.14159265f;
	float roughnessActual = roughness * roughness;
	
	return pow(roughnessActual, 2.0f) / (PI * pow(pow(dot(surfaceNormal, microfacetNormal), 2.0f) * (pow(roughnessActual, 2.0f) - 1.0f) + 1.0f, 2.0f));
}

//========================================== MAIN ===================================================
void main()
{
	// Sample deferred shading textures
	vec3 fragPos = texture(samplerPosition, texCoord).rgb;
	vec3 normal = texture(samplerNormal, texCoord).rgb;
	vec4 albedo = texture(samplerAlbedo, texCoord);
	vec4 material = texture(samplerMaterial, texCoord);
	
	gl_FragDepth = texture(samplerDepth, texCoord).b;
	
	if(ubo.imageIndex == 5)
	{	
		// ----- SHADOW MAP CALCULATIONS -----
		float shadow = 1.0f;
		vec2 projectCoords;
		vec4 shadowClip;
		float lightDepth;
		
		// Find the shadow cascade for this fragment
		int cascadeIndex = 0;
		for(int i = 0; i < CASCADE_COUNT; i++)
		{
			shadowClip = ubo.lightViewMatrix[i] * vec4(fragPos, 1.0f);
			projectCoords.x = shadowClip.x / shadowClip.w / 2.0f + 0.5f;
			projectCoords.y = shadowClip.y / shadowClip.w / 2.0f + 0.5f;
			
			if(clamp(projectCoords.x, 0.0f, 1.0f) == projectCoords.x && clamp(projectCoords.y, 0.0f, 1.0f) == projectCoords.y)
			{
				lightDepth = shadowClip.z / shadowClip.w;
				if(lightDepth < -1.0f || lightDepth > 1.0f)
					continue;
				
				cascadeIndex = i;
				break;
			}
		}
		
		// Project the cascade
		shadowClip = ubo.lightViewMatrix[cascadeIndex] * vec4(fragPos, 1.0f);
		projectCoords.x = shadowClip.x / shadowClip.w / 2.0f + 0.5f;
		projectCoords.y = shadowClip.y / shadowClip.w / 2.0f + 0.5f;
		lightDepth = shadowClip.z / shadowClip.w;
		float mapDepth = texture(samplerShadowMap, vec3(projectCoords, cascadeIndex)).r;
		
		if(lightDepth > mapDepth)
			shadow = 0.25f;
		
		shadow = mix(shadow, 1.0f, abs(1.0f - ubo.shadowStrength));
		
		// ----- PHYISCALLY BASED RENDERING CALCULATIONS -----
		vec3 ambientComponent;
		vec3 diffuseComponent;
		vec3 specularComponent;
		vec3 environmentComponent;
		
		// Read the metallic and roughness values
		float metallic = material.r;
		float roughness = material.g;
		
		vec3 lightDir = -ubo.lightDirection;
		vec3 viewDir = normalize(ubo.cameraPosition - fragPos);
		vec3 halfVec = normalize(lightDir + viewDir);
		float nDotL = clamp(dot(normal, lightDir), 0.0f, 1.0f);
		
		roughness = max(roughness, 0.02f);
		
		// Calculate specular component
		specularComponent = CalculateFresnelReflectance(viewDir, halfVec, vec3(roughness)) *
					CalculateSmithGGXGeometryTerm(roughness, nDotL, dot(normal, viewDir)) *
					CalculateNormalDistributionTrowReitz(roughness, normal, halfVec) *
					shadow * nDotL;
		
		// Calculate diffuse component
		diffuseComponent = albedo.rgb * nDotL * shadow * (1.0f - metallic);
		
		// Calculate ambient component
		ambientComponent = ubo.ambientColor.rgb * albedo.rgb;
		
		// Calculate the environment component
		vec3 R = reflect(-viewDir, normal);
		float mipMapLevel = (pow(roughness - 1.0f, 3.0f) + 1.0f) *  4.0f; // TODO: Dynamic number of lods
		vec4 environmentColor = texture(samplerCubeMap, R, mipMapLevel);
		
		vec3 envFactorRoughness = environmentColor.rgb * pow(1.0f - clamp(dot(normal, viewDir), 0.0f, 1.0f), 5.0f) * (1.0f - roughness);
		vec3 envFactorMetallic = environmentColor.rgb * nDotL * metallic * (1.0f - roughness);
		
		environmentComponent = (envFactorRoughness + envFactorMetallic) * shadow;
		
		outColor = vec4(ambientComponent, 1.0f) + vec4(diffuseComponent, 1.0f) + vec4(specularComponent, 1.0f) + vec4(environmentComponent, 1.0f);
		
		// ----- HDR -----
		float exposure = 1.0f;
		
		vec3 toneMapping = vec3(1.0f) - exp(-outColor.rgb * exposure);
		outColor = vec4(toneMapping, 1.0f);	
	}
	else if(ubo.imageIndex == 4)
		outColor = material;
	else if(ubo.imageIndex == 3)
		outColor = albedo;
	else if(ubo.imageIndex == 2)
		outColor = vec4(normal, 1.0f);
	else
		outColor = vec4(fragPos, 1.0f);
}