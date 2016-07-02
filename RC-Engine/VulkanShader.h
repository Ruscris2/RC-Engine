/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanShader.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

class VulkanShader
{
	private:
		VkPipelineShaderStageCreateInfo shaderStages[2];
		VkDescriptorSetLayout descriptorLayout;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;

		VkBuffer uniformBuffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;

		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 clipMatrix;
		glm::mat4 MVP;
	public:
		VulkanShader();
		~VulkanShader();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		void SetActive(VulkanCommandBuffer * commandBuffer);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
		VkPipelineLayout GetPipelineLayout();
};