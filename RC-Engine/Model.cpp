/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Model.h"

Model::Model()
{
	vertexBuffer = VK_NULL_HANDLE;
}

Model::~Model()
{
	vertexBuffer = VK_NULL_HANDLE;
}

bool Model::Init(VulkanDevice * vulkanDevice)
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

	VkBufferCreateInfo bufferCI{};
	bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCI.size = sizeof(triangle);
	bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &bufferCI, VK_NULL_HANDLE, &vertexBuffer);
	if (result != VK_SUCCESS)
		return false;

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), vertexBuffer, &memReq);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &memory);
	if (result != VK_SUCCESS)
		return false;

	bufferInfo.range = memReq.size;
	bufferInfo.offset = 0;

	uint8_t *pData;
	result = vkMapMemory(vulkanDevice->GetDevice(), memory, 0, memReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, triangle, sizeof(triangle));

	vkUnmapMemory(vulkanDevice->GetDevice(), memory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), vertexBuffer, memory, 0);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void Model::Unload(VulkanDevice * vulkanDevice)
{
	vkFreeMemory(vulkanDevice->GetDevice(), memory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vertexBuffer, VK_NULL_HANDLE);
}

void Model::Render(VulkanCommandBuffer * commandBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetCommandBuffer(), 0, 1, &vertexBuffer, offsets);

	vkCmdDraw(commandBuffer->GetCommandBuffer(), 3, 1, 0, 0);
}
