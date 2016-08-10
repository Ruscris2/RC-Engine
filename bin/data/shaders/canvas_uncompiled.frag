#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D samplerTexture;

layout(location = 0) in vec2 texCoord;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = texture(samplerTexture, texCoord);
}