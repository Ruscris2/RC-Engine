/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanBuffer.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanBuffer.h"
#include "LogManager.h"

extern LogManager * gLogManager;

VulkanBuffer::VulkanBuffer()
{
	buffer = VK_NULL_HANDLE;
	memory = VK_NULL_HANDLE;
}

bool VulkanBuffer::Init(VulkanDevice * vulkanDevice, VkBufferUsageFlags usage, const void * dataPtr,
	VkDeviceSize dataSize, bool useStaging, VulkanCommandBuffer * cmdBuffer)
{
	VkResult result;
	uint8_t *pData;
	VkMemoryAllocateInfo allocInfo{};

	stagedBuffer = useStaging;

	if (useStaging == false)
	{
		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.usage = usage;
		bufferCI.size = dataSize;
		bufferCI.queueFamilyIndexCount = 0;
		bufferCI.pQueueFamilyIndices = VK_NULL_HANDLE;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		result = vkCreateBuffer(vulkanDevice->GetDevice(), &bufferCI, VK_NULL_HANDLE, &buffer);
		if (result != VK_SUCCESS)
			return false;

		vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), buffer, &memReq);

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
			return false;

		result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &memory);
		if (result != VK_SUCCESS)
			return false;

		result = vkMapMemory(vulkanDevice->GetDevice(), memory, 0, memReq.size, 0, (void**)&pData);
		if (result != VK_SUCCESS)
			return false;

		memcpy(pData, dataPtr, (size_t)dataSize);

		vkUnmapMemory(vulkanDevice->GetDevice(), memory);

		result = vkBindBufferMemory(vulkanDevice->GetDevice(), buffer, memory, 0);
		if (result != VK_SUCCESS)
			return false;

		bufferInfo.buffer = buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = dataSize;
	}
	else
	{
		if (cmdBuffer == NULL)
		{
			gLogManager->AddMessage("ERROR: Staged buffer creation requires a valid command buffer!");
			return false;
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCI{};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCI.size = dataSize;
		result = vkCreateBuffer(vulkanDevice->GetDevice(), &bufferCI, VK_NULL_HANDLE, &stagingBuffer);
		if (result != VK_SUCCESS)
			return false;

		vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), stagingBuffer, &memReq);

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex))
			return false;

		result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &stagingMemory);
		if (result != VK_SUCCESS)
			return false;

		result = vkMapMemory(vulkanDevice->GetDevice(), stagingMemory, 0, memReq.size, 0, (void**)&pData);
		if (result != VK_SUCCESS)
			return false;

		memcpy(pData, dataPtr, (size_t)dataSize);

		vkUnmapMemory(vulkanDevice->GetDevice(), stagingMemory);

		result = vkBindBufferMemory(vulkanDevice->GetDevice(), stagingBuffer, stagingMemory, 0);
		if (result != VK_SUCCESS)
			return false;

		bufferCI.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		result = vkCreateBuffer(vulkanDevice->GetDevice(), &bufferCI, VK_NULL_HANDLE, &buffer);
		if (result != VK_SUCCESS)
			return false;

		vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), buffer, &memReq);

		allocInfo.allocationSize = memReq.size;
		if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex))
			return false;

		result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &memory);
		if (result != VK_SUCCESS)
			return false;

		result = vkBindBufferMemory(vulkanDevice->GetDevice(), buffer, memory, 0);
		if (result != VK_SUCCESS)
			return false;

		VkBufferCopy copyRegion{};
		copyRegion.size = dataSize;
		vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingBuffer, buffer, 1, &copyRegion);

		vkFreeMemory(vulkanDevice->GetDevice(), stagingMemory, VK_NULL_HANDLE);
		vkDestroyBuffer(vulkanDevice->GetDevice(), stagingBuffer, VK_NULL_HANDLE);
	}

	return true;
}

void VulkanBuffer::Update(VulkanDevice * vulkanDevice, const void * dataPtr, size_t dataSize)
{
	if (stagedBuffer)
	{
		gLogManager->AddMessage("WARNING: Trying to update a staged buffer!");
		return;
	}

	uint8_t * pData;

	vkMapMemory(vulkanDevice->GetDevice(), memory, 0, memReq.size, 0, (void**)&pData);

	memcpy(pData, dataPtr, dataSize);

	vkUnmapMemory(vulkanDevice->GetDevice(), memory);
}

void VulkanBuffer::Unload(VulkanDevice * vulkanDevice)
{
	vkFreeMemory(vulkanDevice->GetDevice(), memory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), buffer, VK_NULL_HANDLE);
}

VkBuffer * VulkanBuffer::GetBuffer()
{
	return &buffer;
}

VkDescriptorBufferInfo * VulkanBuffer::GetBufferInfo()
{
	return &bufferInfo;
}
