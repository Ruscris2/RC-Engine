/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanPipeline.h                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

class VulkanPipeline
{
	private:
		struct Vertex {
			float x, y, z, w;
			float u, v;
		};
		VkVertexInputBindingDescription vertexBinding;
		VkVertexInputAttributeDescription vertexLayout[2];
		VkPipelineCache pipelineCache;
		VkPipeline pipeline;
	public:
		VulkanPipeline();
		~VulkanPipeline();

		bool Init(VulkanDevice * vulkanDevice, VulkanShader * vulkanShader, VulkanSwapchain * vulkanSwapchain);
		void Unload(VulkanDevice * vulkanDevice);
		void SetActive(VulkanCommandBuffer * commandBuffer);
};