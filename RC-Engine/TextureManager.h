/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: TextureManager.h                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <vector>
#include <string>
#include "Texture.h"

class TextureManager
{
	private:
		struct TextureEntry
		{
			std::string filename;
			Texture * texturePtr;
			unsigned int useCount;
		};
		std::vector<TextureEntry> texturesLoaded;
	public:
		Texture * RequestTexture(std::string filename, VulkanDevice * device, VulkanCommandBuffer * cmdBuffer);
		void ReleaseTexture(Texture * texture, VulkanDevice * device);
		size_t GetLoadedTexturesCount();
};