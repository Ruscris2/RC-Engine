/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkinnedShader.h                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "Shader.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

class SkinnedShader : public Shader
{
	private:
		VkPipelineShaderStageCreateInfo shaderStages[2];

	public:
		SkinnedShader();
		~SkinnedShader();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
};