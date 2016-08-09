/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <thread>
#include <atomic>
#include <mutex>

#include "Camera.h"
#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "DefaultShader.h"
#include "SkinnedShader.h"
#include "DeferredShader.h"
#include "WireframeShader.h"
#include "SkydomeShader.h"
#include "VulkanPipeline.h"
#include "VulkanCommandBuffer.h"
#include "Model.h"
#include "SkinnedModel.h"
#include "WireframeModel.h"
#include "Light.h"
#include "RenderDummy.h"
#include "Animation.h"
#include "Physics.h"
#include "Player.h"
#include "Skydome.h"

class SceneManager
{
	private:
		Physics * physics;
		Camera * camera;
		Light * light;

		DefaultShader * defaultShader;
		SkinnedShader * skinnedShader;
		DeferredShader * deferredShader;
		WireframeShader * wireframeShader;
		SkydomeShader * skydomeShader;

		VulkanPipeline * defaultPipeline;
		VulkanPipeline * skinnedPipeline;
		VulkanPipeline * deferredPipeline;
		VulkanPipeline * wireframePipeline;
		VulkanPipeline * skydomePipeline;

		VulkanCommandBuffer * initCommandBuffer;
		VulkanCommandBuffer * deferredCommandBuffer;
		std::vector<VulkanCommandBuffer*> renderCommandBuffers;

		RenderDummy * renderDummy;
		Skydome * skydome;

		Animation * idleAnim;
		Animation * walkAnim;
		Animation * fallAnim;
		Animation * jumpAnim;
		Animation * runAnim;

		std::vector<Model*> modelList;
		SkinnedModel * male;
		Player * player;
	private:
		bool LoadMapFile(std::string filename, VulkanInterface * vulkan);
		bool BuildDefaultPipeline(VulkanInterface * vulkan);
		bool BuildSkinnedPipeline(VulkanInterface * vulkan);
		bool BuildDeferredPipeline(VulkanInterface * vulkan);
		bool BuildWireframePipeline(VulkanInterface * vulkan);
		bool BuildSkydomePipeline(VulkanInterface * vulkan);
	public:
		SceneManager();
		~SceneManager();

		bool Init(VulkanInterface * vulkan);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan);
};