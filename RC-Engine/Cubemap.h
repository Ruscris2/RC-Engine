/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Cubemap.h                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <string>

#include "VulkanCommandBuffer.h"

class Cubemap
{
	private:
		struct MipMap
		{
			unsigned char * data;
			int width;
			int height;
			unsigned int size;
		};
		VkImage textureImage;
		VkImageView textureImageView;
		VkDeviceMemory textureMemory;
		uint32_t mipMapLevels;
	private:
		bool ReadCubeFace(std::string filename, std::vector<MipMap> & faceData);
	public:
		Cubemap();
		~Cubemap();

		bool Init(VulkanDevice * device, VulkanCommandBuffer * cmdBuffer, std::string cubemapDir);
		void Unload(VulkanDevice * vulkanDevice);
		VkImageView * GetImageView();
};