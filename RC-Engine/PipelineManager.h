/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: PipelineManager.h                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "ShadowMaps.h"
#include "VulkanPipeline.h"
#include "Shader.h"

#pragma once

class PipelineManager
{
	private:
		Shader * defaultShader;
		Shader * skinnedShader;
		Shader * deferredShader;
		Shader * wireframeShader;
		Shader * skydomeShader;
		Shader * canvasShader;
		Shader * shadowShader;
		Shader * shadowSkinnedShader;

		VulkanPipeline * defaultPipeline;
		VulkanPipeline * skinnedPipeline;
		VulkanPipeline * deferredPipeline;
		VulkanPipeline * wireframePipeline;
		VulkanPipeline * skydomePipeline;
		VulkanPipeline * canvasPipeline;
		VulkanPipeline * shadowPipeline;
		VulkanPipeline * shadowSkinnedPipeline;
	private:
		bool BuildDefaultPipeline(VulkanInterface * vulkan);
		bool BuildSkinnedPipeline(VulkanInterface * vulkan);
		bool BuildDeferredPipeline(VulkanInterface * vulkan);
		bool BuildWireframePipeline(VulkanInterface * vulkan);
		bool BuildSkydomePipeline(VulkanInterface * vulkan);
		bool BuildCanvasPipeline(VulkanInterface * vulkan);
		bool BuildShadowPipeline(VulkanInterface * vulkan, ShadowMaps * shadowMaps);
	public:
		PipelineManager();

		bool InitUIPipelines(VulkanInterface * vulkan);
		bool InitGamePipelines(VulkanInterface * vulkan, ShadowMaps * shadowMaps);
		void Unload(VulkanInterface * vulkan);

		VulkanPipeline * GetDefault();
		VulkanPipeline * GetSkinned();
		VulkanPipeline * GetDeferred();
		VulkanPipeline * GetWireframe();
		VulkanPipeline * GetSkydome();
		VulkanPipeline * GetCanvas();
		VulkanPipeline * GetShadow();
		VulkanPipeline * GetShadowSkinned();
};