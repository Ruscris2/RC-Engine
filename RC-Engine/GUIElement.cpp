/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: GUIElement.cpp                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "GUIElement.h"
#include "LogManager.h"
#include "StdInc.h"

extern LogManager * gLogManager;

GUIElement::GUIElement()
{
	texture = NULL;
	canvas = NULL;
}

bool GUIElement::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline, std::string filename)
{
	texture = new Texture();
	if (!texture->Init(vulkan->GetVulkanDevice(), cmdBuffer, filename))
	{
		gLogManager->AddMessage("ERROR: Failed to init texture!");
		return false;
	}

	canvas = new Canvas();
	if (!canvas->Init(vulkan, pipeline))
	{
		gLogManager->AddMessage("ERROR: Failed to init canvas!");
		return false;
	}

	return true;
}

void GUIElement::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(canvas, vulkan);
	SAFE_UNLOAD(texture, vulkan->GetVulkanDevice());
}

void GUIElement::Render(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline, int frameBufferId)
{
	canvas->Render(vulkan, cmdBuffer, pipeline, vulkan->GetOrthoMatrix(), texture->GetImageView(), frameBufferId);
}

void GUIElement::SetPosition(float x, float y)
{
	canvas->SetPosition(x, y);
}

void GUIElement::SetDimensions(float width, float height)
{
	canvas->SetDimensions(width, height);
}
