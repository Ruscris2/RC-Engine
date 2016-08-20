/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Shader.cpp                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>

#include "Shader.h"
#include "LogManager.h"

extern LogManager * gLogManager;

Shader::Shader()
{
	shaderStages = NULL;
}

Shader::~Shader()
{
	shaderStages = NULL;
}

bool Shader::Init(VulkanDevice * vulkanDevice, std::string shaderName, bool hasGeometryShader)
{
	VkResult result;
	uint32_t currentShaderStage = 0;

	std::string shaderDir = "data/shaders/";

	stageCount = 2;
	if (hasGeometryShader)
		stageCount++;

	shaderStages = new VkPipelineShaderStageCreateInfo[stageCount];

	for(uint32_t i = 0; i < stageCount; i++)
		shaderStages[i] = {};

	FILE * file = NULL;

	// Vertex shader
	std::string vertexShaderPath = shaderDir + shaderName + "VS.spv";
	file = fopen(vertexShaderPath.c_str(), "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Couldn't find vertex shader file: " + shaderName + "VS.spv");
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

	shaderStages[currentShaderStage].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[currentShaderStage].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[currentShaderStage].pName = "main";
	shaderStages[currentShaderStage].pNext = VK_NULL_HANDLE;
	shaderStages[currentShaderStage].flags = 0;

	result = vkCreateShaderModule(vulkanDevice->GetDevice(), &vertexShaderCI, VK_NULL_HANDLE, &shaderStages[currentShaderStage].module);
	if (result != VK_SUCCESS)
		return false;

	currentShaderStage++;
	delete[] vsBuffer;

	// Fragment shader
	std::string fragmentShaderPath = shaderDir + shaderName + "FS.spv";
	file = fopen(fragmentShaderPath.c_str(), "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Couldn't find fragment shader file: " + shaderName + "FS.spv");
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

	shaderStages[currentShaderStage].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[currentShaderStage].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[currentShaderStage].pName = "main";
	shaderStages[currentShaderStage].pNext = VK_NULL_HANDLE;
	shaderStages[currentShaderStage].flags = 0;

	result = vkCreateShaderModule(vulkanDevice->GetDevice(), &fragmentShaderCI, VK_NULL_HANDLE, &shaderStages[currentShaderStage].module);
	if (result != VK_SUCCESS)
		return false;

	currentShaderStage++;
	delete[] fsBuffer;

	// Geometry shader
	if (hasGeometryShader)
	{
		std::string geometryShaderPath = shaderDir + shaderName + "GS.spv";
		file = fopen(geometryShaderPath.c_str(), "rb");
		if (file == NULL)
		{
			gLogManager->AddMessage("ERROR: Couldn't find geometry shader file: " + shaderName + "GS.spv");
			return false;
		}

		fseek(file, 0, SEEK_END);
		size = ftell(file);
		rewind(file);

		char * gsBuffer = new char[size];
		fread(gsBuffer, 1, size, file);

		fclose(file);
		file = NULL;

		VkShaderModuleCreateInfo geometryShaderCI{};
		geometryShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		geometryShaderCI.codeSize = size;
		geometryShaderCI.pCode = (uint32_t*)gsBuffer;
		geometryShaderCI.pNext = VK_NULL_HANDLE;
		geometryShaderCI.flags = 0;

		shaderStages[currentShaderStage].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[currentShaderStage].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		shaderStages[currentShaderStage].pName = "main";
		shaderStages[currentShaderStage].pNext = VK_NULL_HANDLE;
		shaderStages[currentShaderStage].flags = 0;

		result = vkCreateShaderModule(vulkanDevice->GetDevice(), &geometryShaderCI, VK_NULL_HANDLE, &shaderStages[currentShaderStage].module);
		if (result != VK_SUCCESS)
			return false;

		currentShaderStage++;
		delete[] gsBuffer;
	}

	return true;
}

void Shader::Unload(VulkanDevice * vulkanDevice)
{
	for(uint32_t i = 0; i < stageCount; i++)
		vkDestroyShaderModule(vulkanDevice->GetDevice(), shaderStages[i].module, VK_NULL_HANDLE);
}

VkPipelineShaderStageCreateInfo * Shader::GetShaderStages()
{
	return shaderStages;
}

uint32_t Shader::GetStageCount()
{
	return stageCount;
}
