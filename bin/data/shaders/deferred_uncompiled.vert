#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO
{
	mat4 mvp;
	mat4 worldMatrix;
} ubo;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormals;
layout (location = 3) in vec3 inTangents;
layout (location = 4) in vec3 inBitangents;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec2 outTexCoord;
layout (location = 2) out vec3 outNormals;
layout (location = 3) out mat3 outTangentSpace;

void main()
{
	gl_Position = ubo.mvp * vec4(pos, 1.0f);
	
	// outWorldPos
	vec4 tempPos = vec4(pos, 1.0f);
	outWorldPos = vec3(ubo.worldMatrix * tempPos);
	
	// outTexCoord
	outTexCoord = inTexCoord;
	
	// outNormals
	outNormals = mat3(transpose(inverse(ubo.worldMatrix))) * inNormals;
	
	// outTangentSpace
	vec3 tan = normalize(vec3(ubo.worldMatrix * vec4(inTangents, 0.0f)));
	vec3 bitan = normalize(vec3(ubo.worldMatrix * vec4(inBitangents, 0.0f)));
	vec3 norm = normalize(vec3(ubo.worldMatrix * vec4(inNormals, 0.0f)));
	outTangentSpace = mat3(tan, bitan, norm);
}