/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: BufferManager.h                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <vector>
#include <string>
#include "VulkanBuffer.h"

class BufferManager
{
	private:
		struct BufferEntry
		{
			std::string bufferName;
			VulkanBuffer * bufferPtr;
			unsigned int useCount;
		};
		std::vector<BufferEntry> buffersLoaded;
	public:
		VulkanBuffer * RequestBuffer(std::string bufferName, VulkanDevice * device, VkBufferUsageFlags usage, const void * dataPtr,
			VkDeviceSize dataSize, bool useStaging, VulkanCommandBuffer * cmdBuffer = 0);
		void ReleaseBuffer(VulkanBuffer * buffer, VulkanDevice * device);
		size_t GetLoadedBuffersCount();
};