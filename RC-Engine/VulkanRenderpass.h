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
		VkClearValue * clear;
		int clearCount, depthClearIndex;
	public:
		VulkanRenderpass();
		~VulkanRenderpass();

		bool Init(VulkanDevice * vulkanDevice, VkAttachmentDescription * attachments, int attachmentCount, VkAttachmentReference * attachmentRefs, int attachRefCount, int depthRefIndex);
		void Unload(VulkanDevice * vulkanDevice);
		void BeginRenderpass(VulkanCommandBuffer * commandBuffer, float r, float g, float b, float a, VkFramebuffer frame, VkSubpassContents contents);
		void EndRenderpass(VulkanCommandBuffer * commandBuffer);
		VkRenderPass GetRenderpass();
};