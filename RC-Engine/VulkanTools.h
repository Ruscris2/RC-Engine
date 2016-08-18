/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanTools.h                                        |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "VulkanCommandBuffer.h"

namespace VulkanTools
{
	void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
		VkImageSubresourceRange * range, VulkanCommandBuffer * cmdBuffer, VulkanDevice * device, bool beginAndExecCmdBuffer);
	glm::vec3 Vec3Transform(glm::vec3 vec, glm::mat4 mat);
}