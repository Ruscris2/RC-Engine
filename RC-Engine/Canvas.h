/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Canvas.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "VulkanPipeline.h"

class Canvas
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
		};
		unsigned int vertexCount;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
		};
		VertexUniformBuffer vertexUniformBuffer;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkMemoryRequirements vertexBufferMemReq;
		Vertex * vertexData;

		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;
		VkWriteDescriptorSet write[2];

		VkBuffer vsUniformBuffer;
		VkDeviceMemory vsUniformMemory;
		VkDescriptorBufferInfo vsUniformBufferInfo;
		VkMemoryRequirements vsMemReq;

		VkImageView * lastImageView;
		float posX, posY, width, height;
		bool updateVertexBuffer;

		std::vector<VulkanCommandBuffer*> drawCmdBuffers;
	private:
		void UpdateVertexData();
	public:
		Canvas();
		~Canvas();

		bool Init(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
			glm::mat4 orthoMatrix, VkImageView * imageView, int frameBufferId);
		void SetPosition(float x, float y);
		void SetDimensions(float width, float height);
};