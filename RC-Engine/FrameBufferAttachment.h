/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: FrameBufferAttachment.h                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

class FrameBufferAttachment
{
	private:
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkFormat format;
	public:
		FrameBufferAttachment();
		~FrameBufferAttachment();

		bool Create(VulkanDevice * device, VkFormat format, VkImageUsageFlagBits usage, VulkanCommandBuffer * cmdBuffer,
			uint32_t width, uint32_t height, uint32_t layerCount);
		void Unload(VulkanDevice * device);
		VkFormat GetFormat();
		VkImageView * GetImageView();
		VkImage GetImage();
};