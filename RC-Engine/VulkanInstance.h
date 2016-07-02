/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanInstance.h                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class VulkanInstance
{
	private:
		VkInstance instance;
		std::vector<const char*> instanceLayers;
		std::vector<const char*> instanceExtensions;
	public:
		VulkanInstance();
		~VulkanInstance();

		bool Init();
		void AddInstanceLayer(const char * instanceLayerName);
		void AddInstanceExtension(const char * instanceExtensionName);
		VkInstance GetInstance();
};