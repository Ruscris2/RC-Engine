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
#include "Texture.h"

class Model
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
		};
		unsigned int vertexCount;
		unsigned int indexCount;

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

		bool Init(std::string filename, VulkanInterface * vulkan, VulkanShader * shader, Texture * texture);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanShader * shader, Camera * camera);
		void SetPosition(float x, float y, float z);
};