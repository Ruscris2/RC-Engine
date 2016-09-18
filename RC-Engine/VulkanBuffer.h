/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanBuffer.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

#pragma once

class VulkanBuffer
{
	private:
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;
		VkMemoryRequirements memReq;
		bool stagedBuffer;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
	public:
		VulkanBuffer();

		bool Init(VulkanDevice * vulkanDevice, VkBufferUsageFlags usage, const void * dataPtr,
			VkDeviceSize dataSize, bool useStaging, VulkanCommandBuffer * cmdBuffer = NULL);
		void Update(VulkanDevice * vulkanDevice, const void * dataPtr, size_t dataSize);
		void Unload(VulkanDevice * vulkanDevice);
		VkBuffer * GetBuffer();
		VkDescriptorBufferInfo * GetBufferInfo();
};