/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "VulkanCommandBuffer.h"
#include "VulkanShader.h"
#include "Camera.h"

class Model
{
	private:
		struct Vertex {
			float x, y, z, w;
			float r, g, b, a;
		};
		glm::mat4 positionMatrix;
		glm::mat4 MVP;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;

		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;
		VkBuffer uniformBuffer;
		VkDeviceMemory uniformMemory;
		VkDescriptorBufferInfo uniformBufferInfo;
		VkMemoryRequirements uniformBufferMemoryReq;
	public:
		Model();
		~Model();

		bool Init(VulkanInterface * vulkan, VulkanShader * shader);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanShader * shader, Camera * camera);
		void SetPosition(float x, float y, float z);
};