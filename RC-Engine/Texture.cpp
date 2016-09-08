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
	struct MipMap
	{
		unsigned char * data;
		int width;
		int height;
		unsigned int size;
	};

	VkResult result;
	void * pData;

	std::vector<MipMap> mipMaps;

	FILE * file = fopen(filename.c_str(), "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Texture file not found! (" + filename + ")");
		return false;
	}

	// Read original image (as mipmap level 0)
	MipMap originalImage;
	fread(&originalImage.width, sizeof(unsigned int), 1, file);
	fread(&originalImage.height, sizeof(unsigned int), 1, file);
	fread(&originalImage.size, sizeof(unsigned int), 1, file);

	originalImage.data = new unsigned char[originalImage.size];
	fread(originalImage.data, sizeof(unsigned char), originalImage.size, file);
	mipMaps.push_back(originalImage);

	// Read mipmaps
	fread(&mipMapsCount, sizeof(int), 1, file);

	for (int i = 0; i < mipMapsCount; i++)
	{
		MipMap mipMap;
		fread(&mipMap.width, sizeof(int), 1, file);
		fread(&mipMap.height, sizeof(int), 1, file);
		fread(&mipMap.size, sizeof(unsigned int), 1, file);

		mipMap.data = new unsigned char[mipMap.size];
		fread(mipMap.data, sizeof(unsigned char), mipMap.size, file);

		mipMaps.push_back(mipMap);
	}

	fclose(file);

	unsigned int totalTextureSize = 0;
	for (unsigned int i = 0; i < mipMaps.size(); i++)
		totalTextureSize += mipMaps[i].size;

	// Create an array of bits which stores all of the texture data
	std::vector<unsigned char> textureData;
	for (unsigned int i = 0; i < mipMaps.size(); i++)
		for (unsigned int j = 0; j < mipMaps[i].size; j++)
			textureData.push_back(mipMaps[i].data[j]);

	for (int i = 0; i < mipMaps.size(); i++)
		delete[] mipMaps[i].data;

	VkMemoryRequirements memReq{};
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCI{};
	bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCI.size = totalTextureSize;
	bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	result = vkCreateBuffer(device->GetDevice(), &bufferCI, VK_NULL_HANDLE, &stagingBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(device->GetDevice(), stagingBuffer, &memReq);

	allocInfo.allocationSize = memReq.size;
	if (!device->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(device->GetDevice(), &allocInfo, VK_NULL_HANDLE, &stagingMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindBufferMemory(device->GetDevice(), stagingBuffer, stagingMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(device->GetDevice(), stagingMemory, 0, memReq.size, 0, &pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, textureData.data(), textureData.size());

	vkUnmapMemory(device->GetDevice(), stagingMemory);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (unsigned int level = 0; level < mipMaps.size(); level++)
	{
		VkBufferImageCopy bufferCopyRegion{};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = level;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;
		bufferCopyRegion.imageExtent.width = mipMaps[level].width;
		bufferCopyRegion.imageExtent.height = mipMaps[level].height;
		offset += (uint32_t)mipMaps[level].size;

		bufferCopyRegions.push_back(bufferCopyRegion);
	}

	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCI.mipLevels = (uint32_t)mipMaps.size();
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCI.extent.width = mipMaps[0].width;
	imageCI.extent.height = mipMaps[0].height;
	imageCI.extent.depth = 1;

	result = vkCreateImage(device->GetDevice(), &imageCI, VK_NULL_HANDLE, &textureImage);
	if (result != VK_SUCCESS)
		return false;

	vkGetImageMemoryRequirements(device->GetDevice(), textureImage, &memReq);

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReq.size;

	if (!device->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memAlloc.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(device->GetDevice(), &memAlloc, VK_NULL_HANDLE, &textureMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindImageMemory(device->GetDevice(), textureImage, textureMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = (uint32_t)mipMaps.size();
	range.layerCount = 1;

	cmdBuffer->BeginRecording();
	VulkanTools::SetImageLayout(textureImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		&range, cmdBuffer, device, false);

	vkCmdCopyBufferToImage(cmdBuffer->GetCommandBuffer(), stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		(uint32_t)bufferCopyRegions.size(), bufferCopyRegions.data());

	VulkanTools::SetImageLayout(textureImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		&range, cmdBuffer, device, false);

	cmdBuffer->EndRecording();
	cmdBuffer->Execute(device, NULL, NULL, NULL, true);

	vkFreeMemory(device->GetDevice(), stagingMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(device->GetDevice(), stagingBuffer, VK_NULL_HANDLE);

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
	viewCI.subresourceRange.levelCount = (uint32_t)mipMaps.size();
	result = vkCreateImageView(device->GetDevice(), &viewCI, VK_NULL_HANDLE, &textureImageView);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void Texture::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyImageView(vulkanDevice->GetDevice(), textureImageView, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), textureMemory, VK_NULL_HANDLE);
	vkDestroyImage(vulkanDevice->GetDevice(), textureImage, VK_NULL_HANDLE);
}

VkImageView * Texture::GetImageView()
{
	return &textureImageView;
}

int Texture::GetMipMapCount()
{
	return mipMapsCount;
}
