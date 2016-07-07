/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: DeferredShader.h                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "Shader.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

class DeferredShader : public Shader
{
private:
	VkPipelineShaderStageCreateInfo shaderStages[2];

public:
	DeferredShader();
	~DeferredShader();

	bool Init(VulkanDevice * vulkanDevice);
	void Unload(VulkanDevice * vulkanDevice);
	VkPipelineShaderStageCreateInfo * GetShaderStages();
};