/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanCommandBuffer.h                                |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "VulkanCommandPool.h"

class VulkanCommandBuffer
{
	private:
		VkFence fence;
		VkCommandBuffer commandBuffer;
	public:
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

		bool Init(VulkanDevice * vulkanDevice, VulkanCommandPool * vulkanCommandPool);
		void Unload(VulkanDevice * vulkanDevice, VulkanCommandPool * vulkanCommandPool);
		void BeginRecording();
		void EndRecording();
		void Execute(VulkanDevice * device, VkPipelineStageFlags flags, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore);
		VkCommandBuffer GetCommandBuffer();
};