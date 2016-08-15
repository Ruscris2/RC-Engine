/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: GUIManager.h                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "GUIElement.h"

#pragma once

class GUIManager
{
	private:
		std::string baseDir;
		bool guiEnabled;

		float cursorPosX, cursorPosY;
		GUIElement * cursor;
	public:
		GUIManager();

		bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline);
		void Unload(VulkanInterface * vulkan);
		void Update(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline, int frameBufferId);
		void ToggleGUI(bool toggle);
};
