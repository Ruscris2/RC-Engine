/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "Camera.h"
#include "VulkanInterface.h"
#include "DefaultShader.h"
#include "DeferredShader.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBuffer.h"
#include "Model.h"
#include "Texture.h"
#include "Light.h"
#include "Canvas.h"

class SceneManager
{
	private:
		Camera * camera;
		Light * light;
		DefaultShader * defaultShader;
		DeferredShader * deferredShader;
		VulkanPipeline * defaultPipeline;
		VulkanPipeline * deferredPipeline;
		VulkanCommandBuffer * deferredCommandBuffer;
		VulkanCommandBuffer * renderCommandBuffer;
		Canvas * defaultShaderCanvas;
		Model * model;
		Model * model2;
		Texture * texture;
	private:
		bool BuildDefaultPipeline(VulkanInterface * vulkan);
		bool BuildDeferredPipeline(VulkanInterface * vulkan);
	public:
		SceneManager();
		~SceneManager();

		bool Init(VulkanInterface * vulkan);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan);
};