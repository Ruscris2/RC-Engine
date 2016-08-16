/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: GUIElement.h                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Texture.h"
#include "Canvas.h"

#pragma once

class GUIElement
{
	private:
		Texture * texture;
		Canvas * canvas;
	public:
		GUIElement();

		bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, std::string filename);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline, int frameBufferId);
		void SetPosition(float x, float y);
		void SetDimensions(float width, float height);
};
