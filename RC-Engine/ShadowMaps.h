/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: ShadowMaps.h                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"

#pragma once

class ShadowMaps
{
	private:
		VkFramebuffer framebuffer;
		uint32_t mapWidth, mapHeight;
		FrameBufferAttachment * depthAttachment;
		VulkanRenderpass * renderpass;
		VkSampler sampler;
	public:
		ShadowMaps();

		bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer);
		void Unload(VulkanInterface * vulkan);
		void BeginShadowPass(VulkanCommandBuffer * commandBuffer);
		void EndShadowPass(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer);
		VulkanRenderpass * GetShadowRenderpass();
		VkFramebuffer GetFramebuffer();
		VkImageView * GetImageView();
};