/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: GUIManager.cpp                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "GUIManager.h"
#include "LogManager.h"
#include "StdInc.h"
#include "Input.h"

extern LogManager * gLogManager;
extern Input * gInput;

GUIManager::GUIManager()
{
	cursor = NULL;
	cursorPosX = cursorPosY = 0.5f;
	guiEnabled = false;
}

bool GUIManager::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline)
{
	baseDir = "data/textures/GUI/";

	cursor = new GUIElement();
	if (!cursor->Init(vulkan, cmdBuffer, pipeline, baseDir + "cursor.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init GUI cursor!");
		return false;
	}
	cursor->SetDimensions(0.03f, 0.06f);

	return true;
}

void GUIManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(cursor, vulkan);
}

void GUIManager::Update(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline, int frameBufferId)
{
	if (guiEnabled)
	{
		if (gInput->GetCursorRelativeX() != 0)
			cursorPosX += 0.001f * gInput->GetCursorRelativeX();
		if (gInput->GetCursorRelativeY() != 0)
			cursorPosY += 0.001f * gInput->GetCursorRelativeY();

		// Clamp cursor to screen
		if (cursorPosX > 1.0f)
			cursorPosX = 1.0f;
		else if (cursorPosX < 0.0f)
			cursorPosX = 0.0f;
		if (cursorPosY > 1.0f)
			cursorPosY = 1.0f;
		else if (cursorPosY < 0.0f)
			cursorPosY = 0.0f;

		cursor->SetPosition(cursorPosX, cursorPosY);
		cursor->Render(vulkan, cmdBuffer, pipeline, frameBufferId);
	}
}

void GUIManager::ToggleGUI(bool toggle)
{
	guiEnabled = toggle;
	cursorPosX = cursorPosY = 0.5f;
}
