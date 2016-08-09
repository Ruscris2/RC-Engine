#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform UBO
{
	mat4 mvp;
} ubo;

layout (location = 0) in vec3 pos;

layout (location = 0) out float height;

void main()
{
	height = pos.y;
	gl_Position = ubo.mvp * vec4(pos, 1.0f);
}
