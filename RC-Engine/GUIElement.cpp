/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: GUIElement.cpp                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "GUIElement.h"
#include "LogManager.h"
#include "StdInc.h"
#include "TextureManager.h"

extern LogManager * gLogManager;
extern TextureManager * gTextureManager;

GUIElement::GUIElement()
{
	texture = NULL;
	canvas = NULL;
}

bool GUIElement::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, std::string filename)
{
	texture = gTextureManager->RequestTexture(filename, vulkan->GetVulkanDevice(), cmdBuffer);
	if (texture == nullptr)
		return false;

	canvas = new Canvas();
	if (!canvas->Init(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init canvas!");
		return false;
	}

	return true;
}

void GUIElement::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(canvas, vulkan);
	gTextureManager->ReleaseTexture(texture, vulkan->GetVulkanDevice());
}

void GUIElement::Render(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline,
	Camera * camera, int frameBufferId)
{
	canvas->Render(vulkan, cmdBuffer, pipeline, camera->GetOrthoMatrix(), texture->GetImageView(), frameBufferId);
}

void GUIElement::SetPosition(float x, float y)
{
	canvas->SetPosition(x, y);
}

void GUIElement::SetDimensions(float width, float height)
{
	canvas->SetDimensions(width, height);
}
