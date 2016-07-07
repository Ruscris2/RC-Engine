/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanTools.h                                        |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanCommandBuffer.h"

namespace VulkanTools
{
	void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
		VkImageSubresourceRange * range, VulkanCommandBuffer * cmdBuffer, VulkanDevice * device, bool beginAndExecCmdBuffer);
}