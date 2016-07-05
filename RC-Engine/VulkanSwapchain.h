/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanSwapchain.h                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderpass.h"

class VulkanSwapchain
{
	private:
		struct SwapChainBuffer
		{
			VkImage image;
			VkImageView view;
		};
		VkSwapchainKHR swapChain;
		std::vector<SwapChainBuffer> swapChainBuffers;
		uint32_t currentBuffer = 0;
		VkSemaphore presentCompleteSemaphore, drawCompleteSemaphore;
		VkFramebuffer * frameBuffers;
	public:
		VulkanSwapchain();
		~VulkanSwapchain();

		bool Init(VulkanDevice * vulkanDevice, VkImageView depthImageView, VulkanRenderpass * vulkanRenderpass);
		void Unload(VulkanDevice * vulkanDevice);
		void AcquireNextImage(VulkanDevice * vulkanDevice);
		void Present(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer);
		VkImage GetCurrentImage();
		VkFramebuffer GetCurrentFramebuffer();
};