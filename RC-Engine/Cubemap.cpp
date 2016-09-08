/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Cubemap.cpp                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <vector>

#include "Cubemap.h"
#include "LogManager.h"
#include "VulkanTools.h"

extern LogManager * gLogManager;

Cubemap::Cubemap()
{
	textureImage = VK_NULL_HANDLE;
	textureMemory = VK_NULL_HANDLE;
	textureImageView = VK_NULL_HANDLE;
}

Cubemap::~Cubemap()
{
	textureImageView = VK_NULL_HANDLE;
	textureMemory = VK_NULL_HANDLE;
	textureImage = VK_NULL_HANDLE;
}

bool Cubemap::ReadCubeFace(std::string filename, std::vector<MipMap>& faceData)
{
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
	faceData.push_back(originalImage);

	// Read mipmaps
	int mipMapsCount;
	fread(&mipMapsCount, sizeof(int), 1, file);

	for (int i = 0; i < mipMapsCount; i++)
	{
		MipMap mipMap;
		fread(&mipMap.width, sizeof(int), 1, file);
		fread(&mipMap.height, sizeof(int), 1, file);
		fread(&mipMap.size, sizeof(unsigned int), 1, file);

		mipMap.data = new unsigned char[mipMap.size];
		fread(mipMap.data, sizeof(unsigned char), mipMap.size, file);

		faceData.push_back(mipMap);
	}

	fclose(file);

	if (mipMapLevels == -1)
		mipMapLevels = (uint32_t)faceData.size();
	else
	{
		if (faceData.size() != mipMapLevels)
		{
			gLogManager->AddMessage("ERROR: Every cubemap face must have the same width, height and mipmaps!");
			return false;
		}
	}

	return true;
}

bool Cubemap::Init(VulkanDevice * device, VulkanCommandBuffer * cmdBuffer, std::string cubemapDir)
{
	VkResult result;
	void * pData;
	mipMapLevels = -1;

	std::vector<MipMap> mipMapsRight;
	std::vector<MipMap> mipMapsLeft;
	std::vector<MipMap> mipMapsTop;
	std::vector<MipMap> mipMapsBottom;
	std::vector<MipMap> mipMapsBack;
	std::vector<MipMap> mipMapsFront;

	// Read each cube face
	if (!ReadCubeFace(cubemapDir + "/right.rct", mipMapsRight))
		return false;
	if (!ReadCubeFace(cubemapDir + "/left.rct", mipMapsLeft))
		return false;
	if (!ReadCubeFace(cubemapDir + "/up.rct", mipMapsTop))
		return false;
	if (!ReadCubeFace(cubemapDir + "/down.rct", mipMapsBottom))
		return false;
	if (!ReadCubeFace(cubemapDir + "/back.rct", mipMapsBack))
		return false;
	if (!ReadCubeFace(cubemapDir + "/front.rct", mipMapsFront))
		return false;

	unsigned int totalTextureSize = 0;
	for (unsigned int i = 0; i < mipMapsRight.size(); i++)
		totalTextureSize += mipMapsRight[i].size;
	for (unsigned int i = 0; i < mipMapsLeft.size(); i++)
		totalTextureSize += mipMapsLeft[i].size;
	for (unsigned int i = 0; i < mipMapsTop.size(); i++)
		totalTextureSize += mipMapsTop[i].size;
	for (unsigned int i = 0; i < mipMapsBottom.size(); i++)
		totalTextureSize += mipMapsBottom[i].size;
	for (unsigned int i = 0; i < mipMapsBack.size(); i++)
		totalTextureSize += mipMapsBack[i].size;
	for (unsigned int i = 0; i < mipMapsFront.size(); i++)
		totalTextureSize += mipMapsFront[i].size;

	// Create an array of bits which stores all of the texture data
	std::vector<unsigned char> textureData;
	for (unsigned int i = 0; i < mipMapsRight.size(); i++)
		for (unsigned int j = 0; j < mipMapsRight[i].size; j++)
			textureData.push_back(mipMapsRight[i].data[j]);
	for (unsigned int i = 0; i < mipMapsLeft.size(); i++)
		for (unsigned int j = 0; j < mipMapsLeft[i].size; j++)
			textureData.push_back(mipMapsLeft[i].data[j]);
	for (unsigned int i = 0; i < mipMapsTop.size(); i++)
		for (unsigned int j = 0; j < mipMapsTop[i].size; j++)
			textureData.push_back(mipMapsTop[i].data[j]);
	for (unsigned int i = 0; i < mipMapsBottom.size(); i++)
		for (unsigned int j = 0; j < mipMapsBottom[i].size; j++)
			textureData.push_back(mipMapsBottom[i].data[j]);
	for (unsigned int i = 0; i < mipMapsBack.size(); i++)
		for (unsigned int j = 0; j < mipMapsBack[i].size; j++)
			textureData.push_back(mipMapsBack[i].data[j]);
	for (unsigned int i = 0; i < mipMapsFront.size(); i++)
		for (unsigned int j = 0; j < mipMapsFront[i].size; j++)
			textureData.push_back(mipMapsFront[i].data[j]);

	for (int i = 0; i < mipMapsRight.size(); i++)
		delete[] mipMapsRight[i].data;
	for (int i = 0; i < mipMapsLeft.size(); i++)
		delete[] mipMapsLeft[i].data;
	for (int i = 0; i < mipMapsTop.size(); i++)
		delete[] mipMapsTop[i].data;
	for (int i = 0; i < mipMapsBottom.size(); i++)
		delete[] mipMapsBottom[i].data;
	for (int i = 0; i < mipMapsBack.size(); i++)
		delete[] mipMapsBack[i].data;
	for (int i = 0; i < mipMapsFront.size(); i++)
		delete[] mipMapsFront[i].data;

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

	for (int face = 0; face < 6; face++)
	{
		for (unsigned int level = 0; level < mipMapLevels; level++)
		{
			VkBufferImageCopy bufferCopyRegion{};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			// Every face has the same width, height and mipmap
			bufferCopyRegion.imageExtent.width = mipMapsRight[level].width;
			bufferCopyRegion.imageExtent.height = mipMapsRight[level].height;
			offset += (uint32_t)mipMapsRight[level].size;

			bufferCopyRegions.push_back(bufferCopyRegion);
		}
	}

	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCI.mipLevels = mipMapLevels;
	imageCI.arrayLayers = 6;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCI.extent.width = mipMapsRight[0].width;
	imageCI.extent.height = mipMapsRight[0].height;
	imageCI.extent.depth = 1;
	imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

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
	range.levelCount = mipMapLevels;
	range.layerCount = 6;

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
	viewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewCI.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewCI.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCI.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCI.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCI.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCI.subresourceRange.baseMipLevel = 0;
	viewCI.subresourceRange.baseArrayLayer = 0;
	viewCI.subresourceRange.layerCount = 6;
	viewCI.subresourceRange.levelCount = mipMapLevels;
	result = vkCreateImageView(device->GetDevice(), &viewCI, VK_NULL_HANDLE, &textureImageView);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void Cubemap::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyImageView(vulkanDevice->GetDevice(), textureImageView, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), textureMemory, VK_NULL_HANDLE);
	vkDestroyImage(vulkanDevice->GetDevice(), textureImage, VK_NULL_HANDLE);
}

VkImageView * Cubemap::GetImageView()
{
	return &textureImageView;
}
