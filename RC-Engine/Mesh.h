/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Mesh.h                                               |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "Material.h"

class Mesh
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
			float nx, ny, nz;
		};

		unsigned int vertexCount;
		unsigned int indexCount;

		struct MaterialUniformBuffer
		{
			float materialShininess;
			glm::vec3 padding;
		};
		MaterialUniformBuffer materialUniformBuffer;

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
		Mesh();
		~Mesh();

		bool Init(VulkanInterface * vulkan, FILE * modelFile);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer);
		void UpdateUniformBuffer(VulkanInterface * vulkan);
		void SetMaterial(Material * material);
		VkDescriptorBufferInfo * GetMaterialUniformBufferInfo();
		Material * GetMaterial();
};