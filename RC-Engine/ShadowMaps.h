/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: ShadowMaps.h                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "Camera.h"
#include "WireframeModel.h"
#include "Light.h"

#pragma once

class ShadowMaps
{
	private:
		VkFramebuffer framebuffer;
		uint32_t mapWidth, mapHeight;
		FrameBufferAttachment * depthAttachment;
		VulkanRenderpass * renderpass;
		VkSampler sampler;
		glm::mat4 * orthoMatrices;
		glm::mat4 * viewMatrices;

		bool render;
		WireframeModel * debugCascade1Face1;
		WireframeModel * debugCascade1Face2;
		WireframeModel * debugCascade1Face3;
		WireframeModel * debugCascade1Face4;

		WireframeModel * debugCascade2Face1;
		WireframeModel * debugCascade2Face2;
		WireframeModel * debugCascade2Face3;
		WireframeModel * debugCascade2Face4;

		WireframeModel * debugCascade3Face1;
		WireframeModel * debugCascade3Face2;
		WireframeModel * debugCascade3Face3;
		WireframeModel * debugCascade3Face4;
	public:
		ShadowMaps();

		bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer);
		void Unload(VulkanInterface * vulkan);
		void BeginShadowPass(VulkanCommandBuffer * commandBuffer);
		void EndShadowPass(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer);
		void SetDepthBias(VulkanCommandBuffer * cmdBuffer);
		void UpdatePartitions(VulkanInterface * vulkan, Camera * viewcamera, Light * light);
		void RenderDebug(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline, Camera * camera,
			int framebufferId);
		VulkanRenderpass * GetShadowRenderpass();
		VkFramebuffer GetFramebuffer();
		VkImageView * GetImageView();
		glm::mat4 GetViewMatrix();
		glm::mat4 GetOrthoMatrix();
		VkSampler GetSampler();
		uint32_t GetMapWidth();
		uint32_t GetMapHeight();
};