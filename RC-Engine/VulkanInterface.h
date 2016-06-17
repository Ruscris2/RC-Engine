/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanInterface.h                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <vulkan/vulkan.h>

class VulkanInterface
{
	private:
		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice gpu;
		uint32_t graphicsQueueFamilyIndex;
		VkPhysicalDeviceProperties gpuProperties;

#ifdef _DEBUG
		VkDebugReportCallbackEXT debugReport;
#endif
	private:
		bool InitInstance();
		bool InitDevice();
#ifdef _DEBUG
		bool InitVulkanDebugMode();
		void UnloadVulkanDebugMode();
#endif
	public:
		VulkanInterface();
		~VulkanInterface();

		bool Init();
};