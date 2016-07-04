/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Texture.cpp                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <vector>

#include "Texture.h"
#include "lodepng.h"
#include "LogManager.h"

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

bool Texture::Init(VulkanInterface * vulkan, std::string filename)
{
	VkResult result;
	unsigned int width, height;
	std::vector<unsigned char> pngData;

	unsigned int errorCode = lodepng::decode(pngData, width, height, filename);
	if (errorCode != 0)
	{
		gLogManager->AddMessage("ERROR: PNG file not found/decoded !");
		return false;
	}

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

	result = vkCreateImage(vulkan->GetVulkanDevice()->GetDevice(), &imageCI, VK_NULL_HANDLE, &textureImage);
	if (result != VK_SUCCESS)
		return false;

	VkMemoryRequirements memReq{};
	vkGetImageMemoryRequirements(vulkan->GetVulkanDevice()->GetDevice(), textureImage, &memReq);

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReq.size;

	if (!vulkan->GetVulkanDevice()->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAlloc.memoryTypeIndex))
		return false;


	result = vkAllocateMemory(vulkan->GetVulkanDevice()->GetDevice(), &memAlloc, VK_NULL_HANDLE, &textureMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindImageMemory(vulkan->GetVulkanDevice()->GetDevice(), textureImage, textureMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	VkImageSubresource subresource{};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkSubresourceLayout subresourceLayout;
	void * pData;

	vkGetImageSubresourceLayout(vulkan->GetVulkanDevice()->GetDevice(), textureImage, &subresource, &subresourceLayout);

	result = vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), textureMemory, 0, memReq.size, 0, &pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, pngData.data(), pngData.size());

	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), textureMemory);

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.layerCount = 1;

	vulkan->SetImageLayout(textureImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &range);

	// Sampler
	VkSamplerCreateInfo samplerCI{};
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.compareOp = VK_COMPARE_OP_NEVER;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 0.0f;
	samplerCI.maxAnisotropy = 8;
	samplerCI.anisotropyEnable = VK_TRUE;
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	result = vkCreateSampler(vulkan->GetVulkanDevice()->GetDevice(), &samplerCI, VK_NULL_HANDLE, &sampler);
	if (result != VK_SUCCESS)
		return false;

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
	result = vkCreateImageView(vulkan->GetVulkanDevice()->GetDevice(), &viewCI, VK_NULL_HANDLE, &textureImageView);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void Texture::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyImageView(vulkanDevice->GetDevice(), textureImageView, VK_NULL_HANDLE);
	vkDestroySampler(vulkanDevice->GetDevice(), sampler, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), textureMemory, VK_NULL_HANDLE);
	vkDestroyImage(vulkanDevice->GetDevice(), textureImage, VK_NULL_HANDLE);
}

VkSampler Texture::GetSampler()
{
	return sampler;
}

VkImageView Texture::GetImageView()
{
	return textureImageView;
}
