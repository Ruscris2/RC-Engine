/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: WireframeShader.h                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Shader.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

#pragma once

class WireframeShader : public Shader
{
	private:
		VkPipelineShaderStageCreateInfo shaderStages[2];

	public:
		WireframeShader();
		~WireframeShader();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		VkPipelineShaderStageCreateInfo * GetShaderStages();
};