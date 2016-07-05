/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: DefaultShader.h                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "Shader.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

class DefaultShader : public Shader
{
	private:
		VkPipelineShaderStageCreateInfo shaderStages[2];

	public:
		DefaultShader();
		~DefaultShader();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
};