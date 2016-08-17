/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: ShadowMaps.h                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "Camera.h"

#pragma once

class ShadowMaps
{
	private:
		VkFramebuffer framebuffer;
		uint32_t mapWidth, mapHeight;
		FrameBufferAttachment * depthAttachment;
		VulkanRenderpass * renderpass;
		VkSampler sampler;
		Camera * camera;
		glm::mat4 orthoMatrix;
	public:
		ShadowMaps();

		bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer);
		void Unload(VulkanInterface * vulkan);
		void BeginShadowPass(VulkanCommandBuffer * commandBuffer);
		void EndShadowPass(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer);
		void SetDepthBias(VulkanCommandBuffer * cmdBuffer);
		VulkanRenderpass * GetShadowRenderpass();
		VkFramebuffer GetFramebuffer();
		VkImageView * GetImageView();
		Camera * GetCamera();
		glm::mat4 GetOrthoMatrix();
		VkSampler GetSampler();
		uint32_t GetMapWidth();
		uint32_t GetMapHeight();
};