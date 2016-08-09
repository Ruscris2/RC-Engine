/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkydomeShader.h                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Shader.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

#pragma once

class SkydomeShader : public Shader
{
	private:
		VkPipelineShaderStageCreateInfo shaderStages[2];

	public:
		SkydomeShader();
		~SkydomeShader();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
};