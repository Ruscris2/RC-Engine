/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Shader.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <vulkan/vulkan.h>

class Shader
{
	public:
		virtual VkPipelineShaderStageCreateInfo * GetShaderStages() = 0;
};