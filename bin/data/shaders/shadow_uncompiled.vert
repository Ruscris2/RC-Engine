#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO
{
	mat4 mvp;
	mat4 worldMatrix;
} ubo;

layout (location = 0) in vec3 pos;

void main()
{
	gl_Position = ubo.mvp * vec4(pos, 1.0f);
}