/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: FrameBufferAttachment.cpp                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "FrameBufferAttachment.h"
#include "Settings.h"
#include "VulkanTools.h"

extern Settings * gSettings;

FrameBufferAttachment::FrameBufferAttachment()
{
	image = VK_NULL_HANDLE;
	memory = VK_NULL_HANDLE;
	view = VK_NULL_HANDLE;
}

FrameBufferAttachment::~FrameBufferAttachment()
{
	view = VK_NULL_HANDLE;
	memory = VK_NULL_HANDLE;
	image = VK_NULL_HANDLE;
}

bool FrameBufferAttachment::Create(VulkanDevice * device, VkFormat format, VkImageUsageFlagBits usage, VulkanCommandBuffer * cmdBuffer)
{
	VkResult result;
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	this->format = format;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	if (aspectMask == 0)
		return false;

	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = format;
	imageCI.extent.width = gSettings->GetWindowWidth();
	imageCI.extent.height = gSettings->GetWindowHeight();
	imageCI.extent.depth = 1;
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc{};
	VkMemoryRequirements memReq;

	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	result = vkCreateImage(device->GetDevice(), &imageCI, VK_NULL_HANDLE, &image);
	if (result != VK_SUCCESS)
		return false;

	vkGetImageMemoryRequirements(device->GetDevice(), image, &memReq);
	memAlloc.allocationSize = memReq.size;
	if (!device->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAlloc.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(device->GetDevice(), &memAlloc, VK_NULL_HANDLE, &memory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindImageMemory(device->GetDevice(), image, memory, 0);
	if (result != VK_SUCCESS)
		return false;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		VulkanTools::SetImageLayout(image, aspectMask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, NULL, cmdBuffer,
			device, true);
	else
		VulkanTools::SetImageLayout(image, aspectMask, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout, NULL, cmdBuffer, device, true);

	VkImageViewCreateInfo viewCI{};
	viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCI.format = format;
	viewCI.subresourceRange.aspectMask = aspectMask;
	viewCI.subresourceRange.baseMipLevel = 0;
	viewCI.subresourceRange.levelCount = 1;
	viewCI.subresourceRange.baseArrayLayer = 0;
	viewCI.subresourceRange.layerCount = 1;
	viewCI.image = image;

	result = vkCreateImageView(device->GetDevice(), &viewCI, VK_NULL_HANDLE, &view);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void FrameBufferAttachment::Unload(VulkanDevice * device)
{
	vkDestroyImageView(device->GetDevice(), view, VK_NULL_HANDLE);
	vkFreeMemory(device->GetDevice(), memory, VK_NULL_HANDLE);
	vkDestroyImage(device->GetDevice(), image, VK_NULL_HANDLE);
}

VkFormat FrameBufferAttachment::GetFormat()
{
	return format;
}

VkImageView FrameBufferAttachment::GetImageView()
{
	return view;
}

VkImage FrameBufferAttachment::GetImage()
{
	return image;
}
