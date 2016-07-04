/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Texture.h                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <string>

#include "VulkanInterface.h"

class Texture
{
	private:
		VkImage textureImage;
		VkImageView textureImageView;
		VkDeviceMemory textureMemory;
		VkSampler sampler;
	public:
		Texture();
		~Texture();

		bool Init(VulkanInterface * vulkan, std::string filename);
		void Unload(VulkanDevice * vulkanDevice);
		VkSampler GetSampler();
		VkImageView GetImageView();
};