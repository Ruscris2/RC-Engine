/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkinnedMesh.h                                        |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
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
			float tx, ty, tz;
			float bx, by, bz;
		};

		unsigned int vertexCount;
		unsigned int indexCount;

		struct MaterialUniformBuffer
		{
			float materialSpecStrength;
			float materialShininess;
			float hasSpecularMap;
			float hasNormalMap;
		};
		MaterialUniformBuffer materialUniformBuffer;

		VulkanBuffer * vertexBuffer;
		VulkanBuffer * indexBuffer;
		VulkanBuffer * materialUBO;

		Material * material;
	public:
		SkinnedMesh();
		~SkinnedMesh();

		bool Init(VulkanInterface * vulkan, FILE * modelFile);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer);
		void UpdateUniformBuffer(VulkanInterface * vulkan);
		void SetMaterial(Material * material);
		Material * GetMaterial();
		VkDescriptorBufferInfo * GetMaterialBufferInfo();
};