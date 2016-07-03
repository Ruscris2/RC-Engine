/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Model.h"
#include "StdInc.h"

Model::Model()
{
	vertexBuffer = VK_NULL_HANDLE;
}

Model::~Model()
{
	vertexBuffer = VK_NULL_HANDLE;
}

bool Model::Init(VulkanDevice * vulkanDevice, VulkanCommandPool * cmdPool)
{
	VkResult result;

	Vertex triangle[3];
	triangle[0].x = -0.5f;
	triangle[0].y = 0.0f;
	triangle[0].z = 0.0f;
	triangle[0].w = 1.0f;
	triangle[0].r = 1.0f;
	triangle[0].g = 0.0f;
	triangle[0].b = 0.0f;
	triangle[0].a = 0.0f;

	triangle[1].x = 0.0f;
	triangle[1].y = 1.0f;
	triangle[1].z = 0.0f;
	triangle[1].w = 1.0f;
	triangle[1].r = 0.0f;
	triangle[1].g = 0.0f;
	triangle[1].b = 1.0f;
	triangle[1].a = 0.0f;

	triangle[2].x = 0.5f;
	triangle[2].y = 0.0f;
	triangle[2].z = 0.0f;
	triangle[2].w = 1.0f;
	triangle[2].r = 0.0f;
	triangle[2].g = 1.0f;
	triangle[2].b = 0.0f;
	triangle[2].a = 0.0f;

	uint32_t indexData[3];
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;

	VkMemoryRequirements memReq;
	VkMemoryAllocateInfo allocInfo{};
	uint8_t *pData;

	// Vertex buffer
	VkBuffer stagingVertexBuffer;
	VkDeviceMemory stagingVertexMemory;

	VkBufferCreateInfo vertexBufferCI{};
	vertexBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	vertexBufferCI.size = sizeof(triangle);
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

	memcpy(pData, triangle, sizeof(triangle));

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
	indexBufferCI.size = sizeof(indexData);
	
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

	memcpy(pData, indexData, sizeof(indexData));

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

	// Command buffer
	VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
	if (!cmdBuffer->Init(vulkanDevice, cmdPool))
		return false;

	cmdBuffer->BeginRecording();

	VkBufferCopy copyRegion{};
	copyRegion.size = sizeof(triangle);
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingVertexBuffer, vertexBuffer, 1, &copyRegion);
	copyRegion.size = sizeof(indexData);
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingIndexBuffer, indexBuffer, 1, &copyRegion);

	cmdBuffer->EndRecording();
	cmdBuffer->Execute(vulkanDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
	
	SAFE_UNLOAD(cmdBuffer, vulkanDevice, cmdPool);

	vkFreeMemory(vulkanDevice->GetDevice(), stagingVertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingVertexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), stagingIndexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingIndexBuffer, VK_NULL_HANDLE);
	return true;
}

void Model::Unload(VulkanDevice * vulkanDevice)
{
	vkFreeMemory(vulkanDevice->GetDevice(), indexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), indexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), vertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vertexBuffer, VK_NULL_HANDLE);
}

void Model::Render(VulkanCommandBuffer * commandBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetCommandBuffer(), 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer->GetCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer->GetCommandBuffer(), 3, 1, 0, 0, 0);
}
