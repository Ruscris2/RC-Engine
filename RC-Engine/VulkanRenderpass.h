/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanRenderpass.h                                   |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

struct VulkanRenderpassCI
{
	VkAttachmentDescription * attachments;
	VkAttachmentReference * attachmentRefs;
	VkAttachmentReference * depthAttachmentRef;
	int attachmentCount;
	VkSubpassDependency * dependencies;
	int dependenciesCount;
};

class VulkanRenderpass
{
	private:
		VkRenderPass renderPass;
		VkClearValue * clear;
		int clearCount;
	public:
		VulkanRenderpass();
		~VulkanRenderpass();

		bool Init(VulkanDevice * vulkanDevice, VulkanRenderpassCI * renderpassCI);
		void Unload(VulkanDevice * vulkanDevice);
		void BeginRenderpass(VulkanCommandBuffer * commandBuffer, float r, float g, float b, float a, VkFramebuffer frame, VkSubpassContents contents);
		void EndRenderpass(VulkanCommandBuffer * commandBuffer);
		VkRenderPass GetRenderpass();
};