/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanPipeline.h                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "Shader.h"

struct VulkanPipelineCI
{
	std::string pipelineName;
	Shader * shader;
	VulkanRenderpass * vulkanRenderpass;
	VkVertexInputAttributeDescription * vertexLayout;
	uint32_t numVertexLayout;
	VkDescriptorSetLayoutBinding * layoutBindings;
	uint32_t numLayoutBindings;
	size_t strideSize;
	VkDescriptorPoolSize * typeCounts;
	int numColorAttachments;
	bool wireframeEnabled;
	bool backFaceCullingEnabled;
	bool transparencyEnabled;
};

class VulkanPipeline
{
	private:
		VkVertexInputBindingDescription vertexBinding;
		VkDescriptorSetLayout descriptorLayout;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;
		VkPipeline pipeline;

		std::string pipelineName;
	public:
		VulkanPipeline();
		~VulkanPipeline();

		bool Init(VulkanInterface * vulkan, VulkanPipelineCI * pipelineCI);
		void Unload(VulkanDevice * vulkanDevice);
		void SetActive(VulkanCommandBuffer * commandBuffer);
		VkDescriptorSet GetDescriptorSet();
		VkDescriptorSetLayout * GetDescriptorLayout();
		VkPipelineLayout GetPipelineLayout();
		std::string GetPipelineName();
};