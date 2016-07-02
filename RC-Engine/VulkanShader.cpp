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
	descriptorPool = VK_NULL_HANDLE;
}

VulkanShader::~VulkanShader()
{
	descriptorPool = VK_NULL_HANDLE;
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

	// Uniform buffer
	float fov = glm::radians(45.0f);
	if (gSettings->GetWindowWidth() > gSettings->GetWindowHeight())
		fov *= (float)gSettings->GetWindowHeight() / gSettings->GetWindowWidth();

	projectionMatrix = glm::perspective(fov, (float)gSettings->GetWindowWidth() / gSettings->GetWindowHeight(), 0.1f, 100.0f);
	viewMatrix = glm::lookAt(glm::vec3(0, 0, -5.0f), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
	modelMatrix = glm::mat4(1.0f);
	clipMatrix = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f);

	MVP = clipMatrix * projectionMatrix * viewMatrix * modelMatrix;

	VkBufferCreateInfo bufferCI{};
	bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCI.size = sizeof(MVP);
	bufferCI.queueFamilyIndexCount = 0;
	bufferCI.pQueueFamilyIndices = VK_NULL_HANDLE;
	bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &bufferCI, VK_NULL_HANDLE, &uniformBuffer);
	if (result != VK_SUCCESS)
		return false;

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), uniformBuffer, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &memory);
	if (result != VK_SUCCESS)
		return false;

	uint8_t *pData;
	result = vkMapMemory(vulkanDevice->GetDevice(), memory, 0, memReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, &MVP, sizeof(MVP));

	vkUnmapMemory(vulkanDevice->GetDevice(), memory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), uniformBuffer, memory, 0);
	if (result != VK_SUCCESS)
		return false;

	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(MVP);

	// Pipeline layout
	VkDescriptorSetLayoutBinding layoutBinding;
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.bindingCount = 1;
	descriptorLayoutCI.pBindings = &layoutBinding;

	result = vkCreateDescriptorSetLayout(vulkanDevice->GetDevice(), &descriptorLayoutCI, VK_NULL_HANDLE, &descriptorLayout);
	if (result != VK_SUCCESS)
		return false;

	VkPipelineLayoutCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineCI.setLayoutCount = 1;
	pipelineCI.pSetLayouts = &descriptorLayout;

	result = vkCreatePipelineLayout(vulkanDevice->GetDevice(), &pipelineCI, VK_NULL_HANDLE, &pipelineLayout);

	// Descriptor pool
	VkDescriptorPoolSize typeCount;
	typeCount.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCount.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.maxSets = 1;
	descriptorPoolCI.poolSizeCount = 1;
	descriptorPoolCI.pPoolSizes = &typeCount;

	result = vkCreateDescriptorPool(vulkanDevice->GetDevice(), &descriptorPoolCI, VK_NULL_HANDLE, &descriptorPool);
	if (result != VK_SUCCESS)
		return false;

	// Descriptor set
	VkDescriptorSetAllocateInfo descSetAllocInfo[1];
	descSetAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocInfo[0].pNext = NULL;
	descSetAllocInfo[0].descriptorPool = descriptorPool;
	descSetAllocInfo[0].descriptorSetCount = 1;
	descSetAllocInfo[0].pSetLayouts = &descriptorLayout;
	result = vkAllocateDescriptorSets(vulkanDevice->GetDevice(), descSetAllocInfo, &descriptorSet);
	if (result != VK_SUCCESS)
		return false;

	VkWriteDescriptorSet write[1];

	write[0] = {};
	write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[0].pNext = NULL;
	write[0].dstSet = descriptorSet;
	write[0].descriptorCount = 1;
	write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write[0].pBufferInfo = &bufferInfo;
	write[0].dstArrayElement = 0;
	write[0].dstBinding = 0;

	vkUpdateDescriptorSets(vulkanDevice->GetDevice(), 1, write, 0, NULL);

	return true;
}

void VulkanShader::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyDescriptorPool(vulkanDevice->GetDevice(), descriptorPool, VK_NULL_HANDLE);
	vkDestroyPipelineLayout(vulkanDevice->GetDevice(), pipelineLayout, VK_NULL_HANDLE);
	vkDestroyDescriptorSetLayout(vulkanDevice->GetDevice(), descriptorLayout, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), memory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), uniformBuffer, VK_NULL_HANDLE);
	vkDestroyShaderModule(vulkanDevice->GetDevice(), shaderStages[1].module, VK_NULL_HANDLE);
	vkDestroyShaderModule(vulkanDevice->GetDevice(), shaderStages[0].module, VK_NULL_HANDLE);
}

void VulkanShader::SetActive(VulkanCommandBuffer * commandBuffer)
{
	vkCmdBindDescriptorSets(commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
}

VkPipelineShaderStageCreateInfo * VulkanShader::GetShaderStages()
{
	return shaderStages;
}

VkPipelineLayout VulkanShader::GetPipelineLayout()
{
	return pipelineLayout;
}
