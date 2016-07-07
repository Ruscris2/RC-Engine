/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanInterface.h                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderpass.h"
#include "FrameBufferAttachment.h"
#include "VulkanPipeline.h"

class VulkanInterface
{
	private:
		struct
		{
			VkFormat format;
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		} depthImage;

		VulkanInstance * vulkanInstance;
		VulkanDevice * vulkanDevice;
		VulkanCommandPool * vulkanCommandPool;
		VulkanCommandBuffer * initCommandBuffer;
		VulkanSwapchain * vulkanSwapchain;
		VulkanRenderpass * mainRenderPass;
		VulkanRenderpass * deferredRenderPass;

		VkViewport viewport;
		VkRect2D scissor;

		VkSampler colorSampler;
		VkFramebuffer deferredFramebuffer;

		FrameBufferAttachment * positionAtt;
		FrameBufferAttachment * normalAtt;
		FrameBufferAttachment * albedoAtt;
		FrameBufferAttachment * depthAtt;

		glm::mat4 projectionMatrix;
		glm::mat4 orthoMatrix;

		VkSemaphore presentCompleteSemaphore;
		VkSemaphore drawCompleteSemaphore;

#ifdef _DEBUG
		VkDebugReportCallbackEXT debugReport;
#endif
	private:
		bool InitDepthBuffer();
		bool InitColorSampler();
		bool InitDeferredFramebuffer();
	
#ifdef _DEBUG
		bool InitVulkanDebugMode();
		void UnloadVulkanDebugMode();
#endif
	public:
		VulkanInterface();
		~VulkanInterface();

		bool Init(HWND hwnd);
		void BeginScene3D(VulkanCommandBuffer * commandBuffer);
		void EndScene3D(VulkanCommandBuffer * commandBuffer);
		void BeginScene2D(VulkanCommandBuffer * commandBuffer, VulkanPipeline * pipeline);
		void EndScene2D(VulkanCommandBuffer * commandBuffer);
		void InitViewportAndScissors(VulkanCommandBuffer * commandBuffer);
		VulkanCommandPool * GetVulkanCommandPool();
		VulkanDevice * GetVulkanDevice();
		VulkanRenderpass * GetMainRenderpass();
		VulkanRenderpass * GetDeferredRenderpass();
		glm::mat4 GetProjectionMatrix();
		glm::mat4 GetOrthoMatrix();
		VkSampler GetColorSampler();
		FrameBufferAttachment * GetPositionAttachment();
		FrameBufferAttachment * GetNormalAttachment();
		FrameBufferAttachment * GetAlbedoAttachment();
		VkFramebuffer GetDeferredFramebuffer();
};