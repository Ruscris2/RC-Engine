/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanPipeline.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"

class Model
{
	private:
		std::vector<Mesh*> meshes;
		std::vector<Texture*> textures;
		std::vector<Material*> materials;
		std::vector<VulkanCommandBuffer*> drawCmdBuffers;

		float posX, posY, posZ;
		float rotX, rotY, rotZ;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
			glm::mat4 worldMatrix;
		};
		VertexUniformBuffer vertexUniformBuffer;

		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;
		VkWriteDescriptorSet descriptorWrite[4];
		
		VkBuffer vsUniformBuffer;
		VkDeviceMemory vsUniformMemory;
		VkDescriptorBufferInfo vsUniformBufferInfo;
		VkMemoryRequirements vsMemReq;
	private:
		void UpdateWorldMatrix();
	public:
		Model();
		~Model();

		bool Init(std::string filename, VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VulkanCommandBuffer * cmdBuffer);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera);
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
		unsigned int GetMeshCount();
		Material * GetMaterial(int materialId);
};