/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Mesh.cpp                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Mesh.h"
#include "StdInc.h"

Mesh::Mesh()
{
	vertexBuffer = VK_NULL_HANDLE;
	indexBuffer = VK_NULL_HANDLE;
	descriptorPool = VK_NULL_HANDLE;
}

Mesh::~Mesh()
{
	descriptorPool = VK_NULL_HANDLE;
	indexBuffer = VK_NULL_HANDLE;
	vertexBuffer = VK_NULL_HANDLE;
}

bool Mesh::Init(VulkanInterface * vulkan, FILE * modelFile, VulkanPipeline * vulkanPipeline, VkDescriptorBufferInfo vsUniformDesc)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	VkResult result;
	VkMemoryRequirements memReq;
	VkMemoryAllocateInfo allocInfo{};
	uint8_t *pData;

	fread(&vertexCount, sizeof(unsigned int), 1, modelFile);
	fread(&indexCount, sizeof(unsigned int), 1, modelFile);

	Vertex * vertexData = new Vertex[vertexCount];
	uint32_t * indexData = new uint32_t[indexCount];

	fread(vertexData, sizeof(Vertex), vertexCount, modelFile);
	fread(indexData, sizeof(uint32_t), indexCount, modelFile);

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
	if (!cmdBuffer->Init(vulkanDevice, cmdPool, true))
		return false;

	cmdBuffer->BeginRecording();

	VkBufferCopy copyRegion{};
	copyRegion.size = sizeof(Vertex) * vertexCount;
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingVertexBuffer, vertexBuffer, 1, &copyRegion);
	copyRegion.size = sizeof(uint32_t) * indexCount;
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingIndexBuffer, indexBuffer, 1, &copyRegion);

	cmdBuffer->EndRecording();
	cmdBuffer->Execute(vulkanDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, true);

	SAFE_UNLOAD(cmdBuffer, vulkanDevice, cmdPool);

	vkFreeMemory(vulkanDevice->GetDevice(), stagingVertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingVertexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), stagingIndexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingIndexBuffer, VK_NULL_HANDLE);

	delete[] vertexData;
	delete[] indexData;

	// Material uniform buffer
	materialUniformBuffer.materialSpecStrength = 0.0f;
	materialUniformBuffer.materialShininess = 0.0f;
	materialUniformBuffer.padding = glm::vec2();

	// Fragment shader uniform buffer
	VkBufferCreateInfo fsBufferCI{};
	fsBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	fsBufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	fsBufferCI.size = sizeof(materialUniformBuffer);
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

	memcpy(pData, &materialUniformBuffer, sizeof(materialUniformBuffer));

	vkUnmapMemory(vulkanDevice->GetDevice(), fsUniformMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), fsUniformBuffer, fsUniformMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	fsUniformBufferInfo.buffer = fsUniformBuffer;
	fsUniformBufferInfo.offset = 0;
	fsUniformBufferInfo.range = sizeof(materialUniformBuffer);

	// Descriptor pool
	VkDescriptorPoolSize typeCounts[4];
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = 1;
	typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	typeCounts[1].descriptorCount = 1;
	typeCounts[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	typeCounts[2].descriptorCount = 1;
	typeCounts[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[3].descriptorCount = 1;

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

	return true;
}

void Mesh::Unload(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();

	vkDestroyDescriptorPool(vulkanDevice->GetDevice(), descriptorPool, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), fsUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), fsUniformBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), indexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), indexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), vertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vertexBuffer, VK_NULL_HANDLE);
}

void Mesh::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetCommandBuffer(), 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer->GetCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
}

void Mesh::UpdateUniformBuffer(VulkanInterface * vulkan)
{
	materialUniformBuffer.materialSpecStrength = material->GetSpecularStrength();
	materialUniformBuffer.materialShininess = material->GetSpecularShininess();
	materialUniformBuffer.padding = glm::vec2();

	uint8_t *pData;

	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), fsUniformMemory, 0, fsMemReq.size, 0, (void**)&pData);
	memcpy(pData, &materialUniformBuffer, sizeof(materialUniformBuffer));
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), fsUniformMemory);
}

void Mesh::SetMaterial(Material * material)
{
	this->material = material;
}

void Mesh::WriteDescriptorSet(VulkanInterface * vulkan, VkDescriptorBufferInfo vsUniformDesc)
{
	descriptorWrite[0] = {};
	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].pNext = NULL;
	descriptorWrite[0].dstSet = descriptorSet;
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[0].pBufferInfo = &vsUniformDesc;
	descriptorWrite[0].dstArrayElement = 0;
	descriptorWrite[0].dstBinding = 0;

	// Write mesh diffuse texture
	VkDescriptorImageInfo diffuseTextureDesc{};
	diffuseTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	diffuseTextureDesc.imageView = *material->GetDiffuseTexture()->GetImageView();
	diffuseTextureDesc.sampler = vulkan->GetColorSampler();

	descriptorWrite[1] = {};
	descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].pNext = NULL;
	descriptorWrite[1].dstSet = descriptorSet;
	descriptorWrite[1].descriptorCount = 1;
	descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[1].pImageInfo = &diffuseTextureDesc;
	descriptorWrite[1].dstArrayElement = 0;
	descriptorWrite[1].dstBinding = 1;

	// Write mesh specular texture
	VkDescriptorImageInfo specularTextureDesc{};
	specularTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	specularTextureDesc.imageView = *material->GetSpecularTexture()->GetImageView();
	specularTextureDesc.sampler = vulkan->GetColorSampler();

	descriptorWrite[2] = {};
	descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[2].pNext = NULL;
	descriptorWrite[2].dstSet = descriptorSet;
	descriptorWrite[2].descriptorCount = 1;
	descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[2].pImageInfo = &specularTextureDesc;
	descriptorWrite[2].dstArrayElement = 0;
	descriptorWrite[2].dstBinding = 2;

	// Update material uniform buffer
	descriptorWrite[3] = {};
	descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[3].pNext = NULL;
	descriptorWrite[3].dstSet = descriptorSet;
	descriptorWrite[3].descriptorCount = 1;
	descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[3].pBufferInfo = &fsUniformBufferInfo;
	descriptorWrite[3].dstArrayElement = 0;
	descriptorWrite[3].dstBinding = 3;

	vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);
}

VkDescriptorSet * Mesh::GetDescriptorSet()
{
	return &descriptorSet;
}

Material * Mesh::GetMaterial()
{
	return material;
}
