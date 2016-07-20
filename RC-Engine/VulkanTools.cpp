/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanTools.cpp                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanTools.h"

void VulkanTools::SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
	VkImageSubresourceRange * range, VulkanCommandBuffer * cmdBuffer, VulkanDevice * device, bool beginAndExecCmdBuffer)
{
	if(beginAndExecCmdBuffer)
		cmdBuffer->BeginRecording();

	VkImageMemoryBarrier imageMemBarrier{};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.oldLayout = oldImageLayout;
	imageMemBarrier.newLayout = newImageLayout;
	imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.image = image;

	if (range == NULL)
	{
		imageMemBarrier.subresourceRange.aspectMask = aspectMask;
		imageMemBarrier.subresourceRange.baseMipLevel = 0;
		imageMemBarrier.subresourceRange.levelCount = 1;
		imageMemBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemBarrier.subresourceRange.layerCount = 1;
	}
	else
		imageMemBarrier.subresourceRange = *range;

	switch (oldImageLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			imageMemBarrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			imageMemBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			imageMemBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			imageMemBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			imageMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			break;
		case VK_IMAGE_LAYOUT_RANGE_SIZE:
			break;
		case VK_IMAGE_LAYOUT_MAX_ENUM:
			break;
		default:
			break;
	}

	switch (newImageLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		{
			imageMemBarrier.srcAccessMask = imageMemBarrier.srcAccessMask;
			imageMemBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			imageMemBarrier.dstAccessMask = imageMemBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		{
			imageMemBarrier.srcAccessMask = imageMemBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
			imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			break;
		case VK_IMAGE_LAYOUT_RANGE_SIZE:
			break;
		case VK_IMAGE_LAYOUT_MAX_ENUM:
			break;
		default:
			break;
	}

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(cmdBuffer->GetCommandBuffer(), srcStages, destStages, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &imageMemBarrier);
	
	if (beginAndExecCmdBuffer)
	{
		cmdBuffer->EndRecording();
		cmdBuffer->Execute(device, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, NULL, NULL, true);
	}
}