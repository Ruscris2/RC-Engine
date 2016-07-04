/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanDevice.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanInstance.h"

class VulkanDevice
{
	private:
		VkPhysicalDevice gpu;
		VkPhysicalDeviceProperties gpuProperties;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		std::vector<VkQueueFamilyProperties> queueFamiliyProperties;
		VkSurfaceKHR surface;
		uint32_t graphicsQueueFamilyIndex;
		VkFormat format;
		VkQueue deviceQueue;
		VkDevice device;
		std::vector<const char*> deviceExtensions;
	public:
		VulkanDevice();
		~VulkanDevice();

		bool Init(VulkanInstance * vulkanInstance, HWND hwnd);
		void Unload(VulkanInstance * vulkanInstance);
		void AddDeviceExtension(const char * deviceExtensionName);
		bool MemoryTypeFromProperties(uint32_t typeBits, VkFlags reqMask, uint32_t * typeIndex);
		VkDevice GetDevice();
		VkPhysicalDevice GetGPU();
		VkQueue GetQueue();
		uint32_t GetGraphicsQueueFamilyIndex();
		VkSurfaceKHR GetSurface();
		VkFormat GetFormat();
		VkPhysicalDeviceProperties GetGPUProperties();
};