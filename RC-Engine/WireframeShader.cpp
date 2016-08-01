/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: WireframeShader.cpp                                  |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>

#include "WireframeShader.h"
#include "LogManager.h"

extern LogManager * gLogManager;

WireframeShader::WireframeShader()
{
	shaderStages[0].module = VK_NULL_HANDLE;
	shaderStages[1].module = VK_NULL_HANDLE;
}

WireframeShader::~WireframeShader()
{
	shaderStages[1].module = VK_NULL_HANDLE;
	shaderStages[0].module = VK_NULL_HANDLE;
}

bool WireframeShader::Init(VulkanDevice * vulkanDevice)
{
	VkResult result;

	shaderStages[0] = {};
	shaderStages[1] = {};

	// Vertex shader
	FILE * file = NULL;
	file = fopen("data/shaders/wireframeVS.spv", "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Couldn't find vertex shader file: wireframeVS.spv");
		return false;
	}

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);

	char * vsBuffer = new char[size];
	fread(vsBuffer, 1, size, file);

	fclose(file);
	file = NULL;

	VkShaderModuleCreateInfo vertexShaderCI{};
	vertexShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexShaderCI.codeSize = size;
	vertexShaderCI.pCode = (uint32_t*)vsBuffer;
	vertexShaderCI.pNext = VK_NULL_HANDLE;
	vertexShaderCI.flags = 0;

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].pName = "main";
	shaderStages[0].pNext = VK_NULL_HANDLE;
	shaderStages[0].flags = 0;

	result = vkCreateShaderModule(vulkanDevice->GetDevice(), &vertexShaderCI, VK_NULL_HANDLE, &shaderStages[0].module);
	if (result != VK_SUCCESS)
		return false;

	// Fragment shader
	file = fopen("data/shaders/wireframeFS.spv", "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Couldn't find vertex shader file: wireframeFS.spv");
		return false;
	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);

	char * fsBuffer = new char[size];
	fread(fsBuffer, 1, size, file);

	fclose(file);
	file = NULL;

	VkShaderModuleCreateInfo fragmentShaderCI{};
	fragmentShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragmentShaderCI.codeSize = size;
	fragmentShaderCI.pCode = (uint32_t*)fsBuffer;
	fragmentShaderCI.pNext = VK_NULL_HANDLE;
	fragmentShaderCI.flags = 0;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].pName = "main";
	shaderStages[1].pNext = VK_NULL_HANDLE;
	shaderStages[1].flags = 0;

	result = vkCreateShaderModule(vulkanDevice->GetDevice(), &fragmentShaderCI, VK_NULL_HANDLE, &shaderStages[1].module);
	if (result != VK_SUCCESS)
		return false;

	delete[] vsBuffer;
	delete[] fsBuffer;

	return true;
}

void WireframeShader::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyShaderModule(vulkanDevice->GetDevice(), shaderStages[1].module, VK_NULL_HANDLE);
	vkDestroyShaderModule(vulkanDevice->GetDevice(), shaderStages[0].module, VK_NULL_HANDLE);
}

VkPipelineShaderStageCreateInfo * WireframeShader::GetShaderStages()
{
	return shaderStages;
}
