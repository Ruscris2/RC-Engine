/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Shader.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"

class Shader
{
	private:
		VkPipelineShaderStageCreateInfo shaderStages[2];

	public:
		Shader();
		~Shader();

		bool Init(VulkanDevice * vulkanDevice, std::string shaderName);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
};