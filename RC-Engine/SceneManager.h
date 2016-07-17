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
#include "SkinnedShader.h"
#include "DeferredShader.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBuffer.h"
#include "Model.h"
#include "SkinnedModel.h"
#include "Light.h"
#include "Canvas.h"
#include "Animation.h"

class SceneManager
{
	private:
		Camera * camera;
		Light * light;
		DefaultShader * defaultShader;
		SkinnedShader * skinnedShader;
		DeferredShader * deferredShader;
		VulkanPipeline * defaultPipeline;
		VulkanPipeline * skinnedPipeline;
		VulkanPipeline * deferredPipeline;
		VulkanCommandBuffer * deferredCommandBuffer;
		VulkanCommandBuffer * renderCommandBuffer;
		Canvas * defaultShaderCanvas;
		Model * model;
		SkinnedModel * male;
		Animation * idleAnim;

	private:
		bool BuildDefaultPipeline(VulkanInterface * vulkan);
		bool BuildSkinnedPipeline(VulkanInterface * vulkan);
		bool BuildDeferredPipeline(VulkanInterface * vulkan);
	public:
		SceneManager();
		~SceneManager();

		bool Init(VulkanInterface * vulkan);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan);
};