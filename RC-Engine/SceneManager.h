/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "Camera.h"
#include "VulkanInterface.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBuffer.h"
#include "Model.h"

class SceneManager
{
	private:
		Camera * camera;
		VulkanShader * vulkanShader;
		VulkanPipeline * vulkanPipeline;
		VulkanCommandBuffer * renderCommandBuffer;
		Model * model;
		Model * model2;
	public:
		SceneManager();
		~SceneManager();

		bool Init(VulkanInterface * vulkan);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan);
};