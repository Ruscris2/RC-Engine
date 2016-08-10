/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: CanvasShader.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Shader.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

#pragma once

class CanvasShader : public Shader
{
	private:
		VkPipelineShaderStageCreateInfo shaderStages[2];

	public:
		CanvasShader();
		~CanvasShader();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
};