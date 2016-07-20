/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Texture.h                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <string>

#include "VulkanCommandBuffer.h"

class Texture
{
	private:
		VkImage textureImage;
		VkImageView textureImageView;
		VkDeviceMemory textureMemory;
		int mipMapsCount;
	public:
		Texture();
		~Texture();

		bool Init(VulkanDevice * device, VulkanCommandBuffer * cmdBuffer, std::string filename);
		void Unload(VulkanDevice * vulkanDevice);
		VkImageView GetImageView();
		int GetMipMapCount();
};