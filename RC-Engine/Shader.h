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
		VkPipelineShaderStageCreateInfo * shaderStages;
		uint32_t stageCount;
	public:
		Shader();
		~Shader();

		bool Init(VulkanDevice * vulkanDevice, std::string shaderName, bool hasGeometryShader);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
		uint32_t GetStageCount();
};