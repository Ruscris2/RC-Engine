/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: WireframeModel.h                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Camera.h"

#pragma once

enum GEOMETRY_TYPE
{
	GEOMETRY_TYPE_INVALID,
	GEOMETRY_TYPE_BOX,
	GEOMETRY_TYPE_SPHERE,
	GEOMETRY_TYPE_POLYGON,
	GEOMETRY_TYPE_CYLINDER
};

struct GEOMETRY_GENERATE_INFO
{
	GEOMETRY_TYPE type;
	float width, height, length;
	float radius;
	unsigned int slices, stacks;
	glm::vec3 v1, v2, v3, v4;
};

class WireframeModel
{
	private:
		struct Vertex {
			float x, y, z;
			float r, g, b, a;
		};

		unsigned int vertexCount;
		unsigned int indexCount;

		VulkanBuffer * vertexBuffer;
		VulkanBuffer * indexBuffer;

		float posX, posY, posZ;
		float rotX, rotY, rotZ;
		glm::mat4 worldMatrix;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
		};
		VertexUniformBuffer vertexUniformBuffer;
		VulkanBuffer * vsUBO;

		std::vector<VulkanCommandBuffer*> drawCmdBuffers;
	private:
		void UpdateWorldMatrix();
		void UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * pipeline);
	public:
		WireframeModel();
		~WireframeModel();

		bool Init(VulkanInterface * vulkan, GEOMETRY_GENERATE_INFO generateInfo, glm::vec4 color);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * pipeline, Camera * camera, int framebufferId);
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
};