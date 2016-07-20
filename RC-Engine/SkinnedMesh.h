/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkinnedMesh.h                                        |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "Material.h"

class SkinnedMesh
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
			float nx, ny, nz;
			float boneWeights[4];
			uint32_t boneIDs[4];
		};

		unsigned int vertexCount;
		unsigned int indexCount;

		struct MaterialUniformBuffer
		{
			float materialSpecStrength;
			float materialShininess;
			glm::vec2 padding;
		};
		MaterialUniformBuffer materialUniformBuffer;

		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;
		VkWriteDescriptorSet descriptorWrite[4];

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;

		VkBuffer fsUniformBuffer;
		VkDeviceMemory fsUniformMemory;
		VkDescriptorBufferInfo fsUniformBufferInfo;
		VkMemoryRequirements fsMemReq;

		Material * material;
	public:
		SkinnedMesh();
		~SkinnedMesh();

		bool Init(VulkanInterface * vulkan, FILE * modelFile, VulkanPipeline * vulkanPipeline, VkDescriptorBufferInfo vsUniformDesc);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer);
		void UpdateUniformBuffer(VulkanInterface * vulkan);
		void SetMaterial(Material * material);
		void WriteDescriptorSet(VulkanInterface * vulkan, VkDescriptorBufferInfo vsUniformDesc);
		VkDescriptorSet * GetDescriptorSet();
		Material * GetMaterial();
};