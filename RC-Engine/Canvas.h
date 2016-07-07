/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Canvas.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "Light.h"

class Canvas
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
		};
		unsigned int vertexCount;
		unsigned int indexCount;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
		};
		struct FragmentUniformBuffer
		{
			glm::vec4 ambientColor;
			glm::vec4 diffuseColor;
			glm::vec3 lightDirection;
			int imageIndex;
		};
		VertexUniformBuffer vertexUniformBuffer;
		FragmentUniformBuffer fragmentUniformBuffer;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;

		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;

		VkBuffer vsUniformBuffer;
		VkDeviceMemory vsUniformMemory;
		VkDescriptorBufferInfo vsUniformBufferInfo;
		VkMemoryRequirements vsMemReq;

		VkBuffer fsUniformBuffer;
		VkDeviceMemory fsUniformMemory;
		VkDescriptorBufferInfo fsUniformBufferInfo;
		VkMemoryRequirements fsMemReq;
	public:
		Canvas();
		~Canvas();

		bool Init(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VkImageView positionView, VkImageView normalView, VkImageView albedoView);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, glm::mat4 orthoMatrix, Light * light, int imageIndex);
};