/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanInterface.h                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "Model.h"

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
		VulkanCommandBuffer * renderCommandBuffer;
		VulkanSwapchain * vulkanSwapchain;
		VulkanShader * vulkanShader;
		VulkanPipeline * vulkanPipeline;

		VkViewport viewport;
		VkRect2D scissor;

		Model * model;
#ifdef _DEBUG
		VkDebugReportCallbackEXT debugReport;
#endif
	private:
		bool InitDepthBuffer();
		void InitViewportAndScissors();
		void SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
#ifdef _DEBUG
		bool InitVulkanDebugMode();
		void UnloadVulkanDebugMode();
#endif
	public:
		VulkanInterface();
		~VulkanInterface();

		bool Init(HWND hwnd);
		void Render();
};