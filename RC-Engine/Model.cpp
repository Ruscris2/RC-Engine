/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Model.h"
#include "StdInc.h"
#include "LogManager.h"

extern LogManager * gLogManager;

Model::Model()
{
	vertexBuffer = VK_NULL_HANDLE;
	indexBuffer = VK_NULL_HANDLE;
	descriptorPool = VK_NULL_HANDLE;
	vsUniformBuffer = VK_NULL_HANDLE;
	fsUniformBuffer = VK_NULL_HANDLE;
}

Model::~Model()
{
	fsUniformBuffer = VK_NULL_HANDLE;
	vsUniformBuffer = VK_NULL_HANDLE;
	descriptorPool = VK_NULL_HANDLE;
	indexBuffer = VK_NULL_HANDLE;
	vertexBuffer = VK_NULL_HANDLE;
}

bool Model::Init(std::string filename, VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, Texture * texture, Light * light)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	VkResult result;
	
	FILE * file = fopen(filename.c_str(), "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Model file not found!");
		return false;
	}

	fread(&vertexCount, sizeof(unsigned int), 1, file);
	fread(&indexCount, sizeof(unsigned int), 1, file);

	Vertex * vertexData = new Vertex[vertexCount];
	uint32_t * indexData = new uint32_t[indexCount];
	
	fread(vertexData, sizeof(Vertex), vertexCount, file);
	fread(indexData, sizeof(uint32_t), indexCount, file);

	fclose(file);
	
	VkMemoryRequirements memReq;
	VkMemoryAllocateInfo allocInfo{};
	uint8_t *pData;

	// Vertex buffer
	VkBuffer stagingVertexBuffer;
	VkDeviceMemory stagingVertexMemory;

	VkBufferCreateInfo vertexBufferCI{};
	vertexBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	vertexBufferCI.size = sizeof(Vertex) * vertexCount;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vertexBufferCI, VK_NULL_HANDLE, &stagingVertexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), stagingVertexBuffer, &memReq);

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &stagingVertexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), stagingVertexMemory, 0, memReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, vertexData, sizeof(Vertex) * vertexCount);

	vkUnmapMemory(vulkanDevice->GetDevice(), stagingVertexMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), stagingVertexBuffer, stagingVertexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	vertexBufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vertexBufferCI, VK_NULL_HANDLE, &vertexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), vertexBuffer, &memReq);

	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &vertexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), vertexBuffer, vertexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	// Index buffer
	VkBuffer stagingIndexBuffer;
	VkDeviceMemory stagingIndexMemory;

	VkBufferCreateInfo indexBufferCI{};
	indexBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	indexBufferCI.size = sizeof(uint32_t) * indexCount;
	
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &indexBufferCI, VK_NULL_HANDLE, &stagingIndexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), stagingIndexBuffer, &memReq);

	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &stagingIndexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), stagingIndexMemory, 0, memReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, indexData, sizeof(uint32_t) * indexCount);

	vkUnmapMemory(vulkanDevice->GetDevice(), stagingIndexMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), stagingIndexBuffer, stagingIndexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	indexBufferCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &indexBufferCI, VK_NULL_HANDLE, &indexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), indexBuffer, &memReq);

	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &indexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), indexBuffer, indexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	// Copy data to VRAM using command buffer
	VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
	if (!cmdBuffer->Init(vulkanDevice, cmdPool))
		return false;

	cmdBuffer->BeginRecording();

	VkBufferCopy copyRegion{};
	copyRegion.size = sizeof(Vertex) * vertexCount;
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingVertexBuffer, vertexBuffer, 1, &copyRegion);
	copyRegion.size = sizeof(uint32_t) * indexCount;
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingIndexBuffer, indexBuffer, 1, &copyRegion);

	cmdBuffer->EndRecording();
	cmdBuffer->Execute(vulkanDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
	
	SAFE_UNLOAD(cmdBuffer, vulkanDevice, cmdPool);

	vkFreeMemory(vulkanDevice->GetDevice(), stagingVertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingVertexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), stagingIndexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingIndexBuffer, VK_NULL_HANDLE);

	delete[] vertexData;
	delete[] indexData;

	// Matrix init
	vertexUniformBuffer.worldMatrix = glm::mat4(1.0f);
	vertexUniformBuffer.MVP = glm::mat4();
	
	// Vertex shader Uniform buffer
	VkBufferCreateInfo vsBufferCI{};
	vsBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vsBufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vsBufferCI.size = sizeof(vertexUniformBuffer);
	vsBufferCI.queueFamilyIndexCount = 0;
	vsBufferCI.pQueueFamilyIndices = VK_NULL_HANDLE;
	vsBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vsBufferCI, VK_NULL_HANDLE, &vsUniformBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), vsUniformBuffer, &vsMemReq);

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = vsMemReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(vsMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &vsUniformMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	vkUnmapMemory(vulkanDevice->GetDevice(), vsUniformMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), vsUniformBuffer, vsUniformMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	vsUniformBufferInfo.buffer = vsUniformBuffer;
	vsUniformBufferInfo.offset = 0;
	vsUniformBufferInfo.range = sizeof(vertexUniformBuffer);
	
	// Fragment shader Uniform buffer
	VkBufferCreateInfo fsBufferCI{};
	fsBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	fsBufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	fsBufferCI.size = light->GetLightBufferSize();
	fsBufferCI.queueFamilyIndexCount = 0;
	fsBufferCI.pQueueFamilyIndices = VK_NULL_HANDLE;
	fsBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &fsBufferCI, VK_NULL_HANDLE, &fsUniformBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), fsUniformBuffer, &fsMemReq);

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = fsMemReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(fsMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &fsUniformMemory);
	if (result != VK_SUCCESS)
		return false;
	
	result = vkMapMemory(vulkanDevice->GetDevice(), fsUniformMemory, 0, fsMemReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, light->GetLightBuffer(), light->GetLightBufferSize());

	vkUnmapMemory(vulkanDevice->GetDevice(), fsUniformMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), fsUniformBuffer, fsUniformMemory, 0);
	if (result != VK_SUCCESS)
		return false;


	fsUniformBufferInfo.buffer = fsUniformBuffer;
	fsUniformBufferInfo.offset = 0;
	fsUniformBufferInfo.range = light->GetLightBufferSize();

	// Descriptor pool
	VkDescriptorPoolSize typeCounts[3];
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = 1;
	typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	typeCounts[1].descriptorCount = 1;
	typeCounts[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[2].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.maxSets = 1;
	descriptorPoolCI.poolSizeCount = sizeof(typeCounts) / sizeof(typeCounts[0]);
	descriptorPoolCI.pPoolSizes = typeCounts;

	result = vkCreateDescriptorPool(vulkanDevice->GetDevice(), &descriptorPoolCI, VK_NULL_HANDLE, &descriptorPool);
	if (result != VK_SUCCESS)
		return false;

	// Descriptor set
	VkDescriptorSetAllocateInfo descSetAllocInfo[1];
	descSetAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocInfo[0].pNext = NULL;
	descSetAllocInfo[0].descriptorPool = descriptorPool;
	descSetAllocInfo[0].descriptorSetCount = 1;
	descSetAllocInfo[0].pSetLayouts = vulkanPipeline->GetDescriptorLayout();
	result = vkAllocateDescriptorSets(vulkanDevice->GetDevice(), descSetAllocInfo, &descriptorSet);
	if (result != VK_SUCCESS)
		return false;

	VkWriteDescriptorSet write[3];

	write[0] = {};
	write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[0].pNext = NULL;
	write[0].dstSet = descriptorSet;
	write[0].descriptorCount = 1;
	write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write[0].pBufferInfo = &vsUniformBufferInfo;
	write[0].dstArrayElement = 0;
	write[0].dstBinding = 0;

	VkDescriptorImageInfo textureDesc{};
	textureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	textureDesc.imageView = texture->GetImageView();
	textureDesc.sampler = texture->GetSampler();

	write[1] = {};
	write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[1].pNext = NULL;
	write[1].dstSet = descriptorSet;
	write[1].descriptorCount = 1;
	write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[1].pImageInfo = &textureDesc;
	write[1].dstArrayElement = 0;
	write[1].dstBinding = 1;

	write[2] = {};
	write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[2].pNext = NULL;
	write[2].dstSet = descriptorSet;
	write[2].descriptorCount = 1;
	write[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write[2].pBufferInfo = &fsUniformBufferInfo;
	write[2].dstArrayElement = 0;
	write[2].dstBinding = 2;

	vkUpdateDescriptorSets(vulkanDevice->GetDevice(), sizeof(write) / sizeof(write[0]), write, 0, NULL);

	return true;
}

void Model::Unload(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	
	vkDestroyDescriptorPool(vulkanDevice->GetDevice(), descriptorPool, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), fsUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), fsUniformBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), vsUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vsUniformBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), indexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), indexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), vertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vertexBuffer, VK_NULL_HANDLE);
}

void Model::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera, Light * light)
{
	// Update vertex uniform buffer
	vertexUniformBuffer.MVP = vulkan->GetProjectionMatrix() * camera->GetViewMatrix() * vertexUniformBuffer.worldMatrix;

	uint8_t *pData;
	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory);

	// Update fragment uniform buffer
	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), fsUniformMemory, 0, fsMemReq.size, 0, (void**)&pData);
	memcpy(pData, light->GetLightBuffer(), light->GetLightBufferSize());
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), fsUniformMemory);

	// Draw
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetCommandBuffer(), 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer->GetCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetPipelineLayout(), 0, 1, &descriptorSet, 0, NULL);

	vkCmdDrawIndexed(commandBuffer->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
}

void Model::SetPosition(float x, float y, float z)
{
	posX = x;
	posY = y;
	posZ = z;
	UpdateWorldMatrix();
}

void Model::SetRotation(float x, float y, float z)
{
	rotX = x;
	rotY = y;
	rotZ = z;
	UpdateWorldMatrix();
}

void Model::UpdateWorldMatrix()
{
	vertexUniformBuffer.worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, posZ));
	vertexUniformBuffer.worldMatrix = glm::rotate(vertexUniformBuffer.worldMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
	vertexUniformBuffer.worldMatrix = glm::rotate(vertexUniformBuffer.worldMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
	vertexUniformBuffer.worldMatrix = glm::rotate(vertexUniformBuffer.worldMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
}