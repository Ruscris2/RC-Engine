/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: TextureManager.cpp                                   |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "TextureManager.h"
#include "LogManager.h"
#include "StdInc.h"

extern LogManager * gLogManager;

Texture * TextureManager::RequestTexture(std::string filename, VulkanDevice * device, VulkanCommandBuffer * cmdBuffer)
{
	// Check if texture is already loaded
	for (unsigned int i = 0; i < texturesLoaded.size(); i++)
	{
		if (filename == texturesLoaded[i].filename)
		{
			texturesLoaded[i].useCount++;
			return texturesLoaded[i].texturePtr;
		}
	}

	// If texture is not loaded, create new entry
	Texture * texture = new Texture();
	if (!texture->Init(device, cmdBuffer, filename))
	{
		gLogManager->AddMessage("ERROR: Couldn't init a texture!");
		return nullptr;
	}

	TextureEntry entry;
	entry.filename = filename;
	entry.texturePtr = texture;
	entry.useCount = 1;
	texturesLoaded.push_back(entry);

	return texture;
}

void TextureManager::ReleaseTexture(Texture * texture, VulkanDevice * device)
{
	for (unsigned int i = 0; i < texturesLoaded.size(); i++)
	{
		if (texture == texturesLoaded[i].texturePtr)
		{
			if (texturesLoaded[i].useCount > 1)
				texturesLoaded[i].useCount--;
			else
			{
				SAFE_UNLOAD(texturesLoaded[i].texturePtr, device);
				texturesLoaded.erase(texturesLoaded.begin() + i);
			}

			texture = NULL;
			break;
		}
	}
}

size_t TextureManager::GetLoadedTexturesCount()
{
	return texturesLoaded.size();
}
