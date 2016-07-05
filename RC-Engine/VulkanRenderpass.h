/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanRenderpass.h                                   |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

class VulkanRenderpass
{
	private:
		VkRenderPass renderPass;
	public:
		VulkanRenderpass();
		~VulkanRenderpass();

		bool Init(VulkanDevice * vulkanDevice, VkAttachmentDescription * attachments, int attachmentCount);
		void Unload(VulkanDevice * vulkanDevice);
		void BeginRenderpass(VulkanCommandBuffer * commandBuffer, float r, float g, float b, float a, VkFramebuffer frame);
		void EndRenderpass(VulkanCommandBuffer * commandBuffer);
		VkRenderPass GetRenderpass();
};