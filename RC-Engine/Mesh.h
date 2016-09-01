/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Mesh.h                                               |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Material.h"

class Mesh
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
			float nx, ny, nz;
			float tx, ty, tz;
			float bx, by, bz;
		};

		unsigned int vertexCount;
		unsigned int indexCount;

		struct MaterialUniformBuffer
		{
			float materialSpecStrength;
			float materialShininess;
			float hasSpecMap;
			float hasNormalMap;
		};
		MaterialUniformBuffer materialUniformBuffer;

		VulkanBuffer * vertexBuffer;
		VulkanBuffer * indexBuffer;
		VulkanBuffer * materialUBO;

		Material * material;
	public:
		Mesh();
		~Mesh();

		bool Init(VulkanInterface * vulkan, FILE * modelFile);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer);
		void SetMaterial(Material * material);
		void UpdateUniformBuffer(VulkanInterface * vulkan);
		Material * GetMaterial();
		VkDescriptorBufferInfo * GetMaterialBufferInfo();
};