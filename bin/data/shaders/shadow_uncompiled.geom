#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define CASCADE_COUNT 3

layout (triangles, invocations = CASCADE_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 1) uniform UBO
{
	mat4 lightViewProj[CASCADE_COUNT];
} ubo;

layout (binding = 2) uniform FrustumBuffer
{
	vec4 frustumCullCascade[CASCADE_COUNT];
} frustumBuffer;

float AcessFrustumCullFloatArray(int index)
{
	return frustumBuffer.frustumCullCascade[index >> 2][index & 0x3];
}

void main()
{
	if(AcessFrustumCullFloatArray(gl_InvocationID) == 1.0f)
	{
		for(int i = 0; i < gl_in.length(); i++)
		{
			gl_Layer = gl_InvocationID;
			gl_Position = ubo.lightViewProj[gl_InvocationID] * gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}