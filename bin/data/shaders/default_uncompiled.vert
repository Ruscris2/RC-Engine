#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO
{
	mat4 mvp;
} ubo;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;

void main()
{
	outTexCoord = inTexCoord;
	gl_Position = ubo.mvp * vec4(pos, 1.0f);
}
