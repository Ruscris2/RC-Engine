/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Texture.cpp                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <vector>

#include "Texture.h"
#include "LogManager.h"
#include "VulkanTools.h"

extern LogManager * gLogManager;

Texture::Texture()
{
	textureImage = VK_NULL_HANDLE;
	textureMemory = VK_NULL_HANDLE;
	textureImageView = VK_NULL_HANDLE;
}

Texture::~Texture()
{
	textureImageView = VK_NULL_HANDLE;
	textureMemory = VK_NULL_HANDLE;
	textureImage = VK_NULL_HANDLE;
}

bool Texture::Init(VulkanDevice * device, VulkanCommandBuffer * cmdBuffer, std::string filename)
{
	VkResult result;
	unsigned char * fileData;
	unsigned int fileSize;
	unsigned int width, height;

	FILE * file = fopen(filename.c_str(), "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Texture file not found!");
		return false;
	}

	fread(&width, sizeof(unsigned int), 1, file);
	fread(&height, sizeof(unsigned int), 1, file);
	fread(&fileSize, sizeof(unsigned int), 1, file);

	fileData = new unsigned char[fileSize];
	fread(fileData, sizeof(unsigned char), fileSize, file);

	fclose(file);

	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_LINEAR;
	imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageCI.extent.width = width;
	imageCI.extent.height = height;
	imageCI.extent.depth = 1;

	result = vkCreateImage(device->GetDevice(), &imageCI, VK_NULL_HANDLE, &textureImage);
	if (result != VK_SUCCESS)
		return false;

	VkMemoryRequirements memReq{};
	vkGetImageMemoryRequirements(device->GetDevice(), textureImage, &memReq);

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReq.size;

	if (!device->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAlloc.memoryTypeIndex))
		return false;


	result = vkAllocateMemory(device->GetDevice(), &memAlloc, VK_NULL_HANDLE, &textureMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindImageMemory(device->GetDevice(), textureImage, textureMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	VkImageSubresource subresource{};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkSubresourceLayout subresourceLayout;
	void * pData;

	vkGetImageSubresourceLayout(device->GetDevice(), textureImage, &subresource, &subresourceLayout);

	result = vkMapMemory(device->GetDevice(), textureMemory, 0, memReq.size, 0, &pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, fileData, fileSize);

	vkUnmapMemory(device->GetDevice(), textureMemory);

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.layerCount = 1;

	VulkanTools::SetImageLayout(textureImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		&range, cmdBuffer, device, true);

	VkImageViewCreateInfo viewCI{};
	viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCI.image = textureImage;
	viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCI.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewCI.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCI.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCI.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCI.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCI.subresourceRange.baseMipLevel = 0;
	viewCI.subresourceRange.baseArrayLayer = 0;
	viewCI.subresourceRange.layerCount = 1;
	viewCI.subresourceRange.levelCount = 1;
	result = vkCreateImageView(device->GetDevice(), &viewCI, VK_NULL_HANDLE, &textureImageView);
	if (result != VK_SUCCESS)
		return false;

	delete[] fileData;
	return true;
}

void Texture::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyImageView(vulkanDevice->GetDevice(), textureImageView, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), textureMemory, VK_NULL_HANDLE);
	vkDestroyImage(vulkanDevice->GetDevice(), textureImage, VK_NULL_HANDLE);
}

VkImageView Texture::GetImageView()
{
	return textureImageView;
}
