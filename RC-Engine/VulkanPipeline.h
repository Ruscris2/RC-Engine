/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanPipeline.h                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "Shader.h"
#include "VulkanRenderpass.h"
#include "VulkanCommandBuffer.h"

class VulkanPipeline
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
			float nx, ny, nz;
		};
		VkVertexInputBindingDescription vertexBinding;
		VkDescriptorSetLayout descriptorLayout;
		VkPipelineLayout pipelineLayout;
		VkPipelineCache pipelineCache;
		VkPipeline pipeline;
	public:
		VulkanPipeline();
		~VulkanPipeline();

		bool Init(VulkanDevice * vulkanDevice, Shader * shader, VulkanRenderpass * vulkanRenderpass);
		void Unload(VulkanDevice * vulkanDevice);
		void SetActive(VulkanCommandBuffer * commandBuffer);
		VkDescriptorSetLayout * GetDescriptorLayout();
		VkPipelineLayout GetPipelineLayout();
};