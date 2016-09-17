/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: BufferManager.cpp                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "BufferManager.h"
#include "StdInc.h"

VulkanBuffer * BufferManager::RequestBuffer(std::string bufferName, VulkanDevice * device, VkBufferUsageFlags usage, const void * dataPtr,
	VkDeviceSize dataSize, bool useStaging, VulkanCommandBuffer * cmdBuffer)
{
	// Check if buffer is already loaded
	for (unsigned int i = 0; i < buffersLoaded.size(); i++)
	{
		if (bufferName == buffersLoaded[i].bufferName)
		{
			buffersLoaded[i].useCount++;
			return buffersLoaded[i].bufferPtr;
		}
	}

	// If buffer is not loaded, create new entry
	VulkanBuffer * buffer = new VulkanBuffer();
	if (!buffer->Init(device, usage, dataPtr, dataSize, useStaging, cmdBuffer))
		return nullptr;

	BufferEntry entry;
	entry.bufferName = bufferName;
	entry.bufferPtr = buffer;
	entry.useCount = 1;
	buffersLoaded.push_back(entry);

	return buffer;
}

void BufferManager::ReleaseBuffer(VulkanBuffer * buffer, VulkanDevice * device)
{
	for (unsigned int i = 0; i < buffersLoaded.size(); i++)
	{
		if (buffer == buffersLoaded[i].bufferPtr)
		{
			if (buffersLoaded[i].useCount > 1)
				buffersLoaded[i].useCount--;
			else
			{
				SAFE_UNLOAD(buffersLoaded[i].bufferPtr, device);
				buffersLoaded.erase(buffersLoaded.begin() + i);
			}

			buffer = nullptr;
			break;
		}
	}
}

size_t BufferManager::GetLoadedBuffersCount()
{
	return buffersLoaded.size();
}
