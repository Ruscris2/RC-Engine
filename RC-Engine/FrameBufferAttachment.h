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

		bool Create(VulkanDevice * device, VkFormat format, VkImageUsageFlagBits usage, VulkanCommandBuffer * cmdBuffer);
		void Unload(VulkanDevice * device);
		VkFormat GetFormat();
		VkImageView * GetImageView();
		VkImage GetImage();
};