/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: ShadowMaps.h                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "Camera.h"
#include "WireframeModel.h"
#include "Sunlight.h"
#include "VulkanBuffer.h"
#include "FrustumCuller.h"

#pragma once

#define SHADOW_CASCADE_COUNT 3

class ShadowMaps
{
	private:
		VkFramebuffer framebuffer;
		uint32_t mapSize;
		FrameBufferAttachment * depthAttachment;
		VulkanRenderpass * renderpass;
		VkSampler sampler;
		glm::mat4 * orthoMatrices;
		glm::mat4 * viewMatrices;
		float depthRadius;
		float frustumRadius;

		glm::mat4 * projectionMatrixPartitions;

		struct GeometryUniformBuffer
		{
			glm::mat4 lightViewProj[SHADOW_CASCADE_COUNT];
		};
		GeometryUniformBuffer geometryUniformBuffer;
		VulkanBuffer * shadowGS_UBO;

		FrustumCuller ** cascadeFrustumCullers;
	public:
		ShadowMaps();

		bool Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, Camera * camera);
		void Unload(VulkanInterface * vulkan);
		void BeginShadowPass(VulkanCommandBuffer * commandBuffer);
		void EndShadowPass(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer);
		void SetDepthBias(VulkanCommandBuffer * cmdBuffer);
		void UpdatePartitions(VulkanInterface * vulkan, Camera * viewcamera, Sunlight * light);
		VulkanRenderpass * GetShadowRenderpass();
		VkFramebuffer GetFramebuffer();
		VkImageView * GetImageView();
		VkDescriptorBufferInfo * GetBufferInfo();
		glm::mat4 GetLightViewProj(int index);
		VkSampler GetSampler();
		uint32_t GetMapSize();
		FrustumCuller * GetFrustumCuller(int index);
};