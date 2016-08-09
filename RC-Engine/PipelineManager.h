/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: PipelineManager.h                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "DefaultShader.h"
#include "SkinnedShader.h"
#include "DeferredShader.h"
#include "WireframeShader.h"
#include "SkydomeShader.h"

#pragma once

class PipelineManager
{
	private:
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
	private:
		bool BuildDefaultPipeline(VulkanInterface * vulkan);
		bool BuildSkinnedPipeline(VulkanInterface * vulkan);
		bool BuildDeferredPipeline(VulkanInterface * vulkan);
		bool BuildWireframePipeline(VulkanInterface * vulkan);
		bool BuildSkydomePipeline(VulkanInterface * vulkan);
	public:
		PipelineManager();

		bool Init(VulkanInterface * vulkan);
		void Unload(VulkanInterface * vulkan);

		VulkanPipeline * GetDefault();
		VulkanPipeline * GetSkinned();
		VulkanPipeline * GetDeferred();
		VulkanPipeline * GetWireframe();
		VulkanPipeline * GetSkydome();
};