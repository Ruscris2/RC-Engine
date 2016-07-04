/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanShader.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>

#include "VulkanShader.h"
#include "LogManager.h"
#include "Settings.h"

extern LogManager * gLogManager;
extern Settings * gSettings;

VulkanShader::VulkanShader()
{
	shaderStages[0].module = VK_NULL_HANDLE;
	shaderStages[1].module = VK_NULL_HANDLE;
	descriptorLayout = VK_NULL_HANDLE;
	pipelineLayout = VK_NULL_HANDLE;
}

VulkanShader::~VulkanShader()
{
	descriptorLayout = VK_NULL_HANDLE;
	shaderStages[1].module = VK_NULL_HANDLE;
	shaderStages[0].module = VK_NULL_HANDLE;
	pipelineLayout = VK_NULL_HANDLE;
}

bool VulkanShader::Init(VulkanDevice * vulkanDevice)
{
	VkResult result;

	shaderStages[0] = {};
	shaderStages[1] = {};

	// Vertex shader
	FILE * file = NULL;
	file = fopen("data/shaders/defaultVS.spv", "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Couldn't find vertex shader file: defaultVS.spv");
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
	file = fopen("data/shaders/defaultFS.spv", "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Couldn't find vertex shader file: defaultFS.spv");
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

	// Pipeline layout
	VkDescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindings[0].pImmutableSamplers = VK_NULL_HANDLE;
	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindings[1].pImmutableSamplers = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.bindingCount = 2;
	descriptorLayoutCI.pBindings = layoutBindings;

	result = vkCreateDescriptorSetLayout(vulkanDevice->GetDevice(), &descriptorLayoutCI, VK_NULL_HANDLE, &descriptorLayout);
	if (result != VK_SUCCESS)
		return false;

	VkPipelineLayoutCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineCI.setLayoutCount = 1;
	pipelineCI.pSetLayouts = &descriptorLayout;

	result = vkCreatePipelineLayout(vulkanDevice->GetDevice(), &pipelineCI, VK_NULL_HANDLE, &pipelineLayout);

	return true;
}

void VulkanShader::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyPipelineLayout(vulkanDevice->GetDevice(), pipelineLayout, VK_NULL_HANDLE);
	vkDestroyDescriptorSetLayout(vulkanDevice->GetDevice(), descriptorLayout, VK_NULL_HANDLE);
	vkDestroyShaderModule(vulkanDevice->GetDevice(), shaderStages[1].module, VK_NULL_HANDLE);
	vkDestroyShaderModule(vulkanDevice->GetDevice(), shaderStages[0].module, VK_NULL_HANDLE);
}

VkPipelineShaderStageCreateInfo * VulkanShader::GetShaderStages()
{
	return shaderStages;
}

VkPipelineLayout VulkanShader::GetPipelineLayout()
{
	return pipelineLayout;
}

VkDescriptorSetLayout * VulkanShader::GetDescriptorLayout()
{
	return &descriptorLayout;
}
