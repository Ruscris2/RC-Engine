/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"

class Model
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
			float nx, ny, nz;
		};
		unsigned int vertexCount;
		unsigned int indexCount;

		float posX, posY, posZ;
		float rotX, rotY, rotZ;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
			glm::mat4 worldMatrix;
		};
		VertexUniformBuffer vertexUniformBuffer;

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
	private:
		void UpdateWorldMatrix();
	public:
		Model();
		~Model();

		bool Init(std::string filename, VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, Texture * texture, Light * light);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera, Light * light);
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
};