/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanCommandPool.h                                  |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"

class VulkanCommandPool
{
	private:
		VkCommandPool commandPool;
	public:
		VulkanCommandPool();
		~VulkanCommandPool();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		VkCommandPool GetCommandPool();
};