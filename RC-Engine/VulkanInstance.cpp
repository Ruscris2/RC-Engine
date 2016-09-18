/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanInstance.cpp                                   |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInstance.h"

VulkanInstance::VulkanInstance()
{
	instance = VK_NULL_HANDLE;
}

VulkanInstance::~VulkanInstance()
{
	vkDestroyInstance(instance, VK_NULL_HANDLE);
	instance = VK_NULL_HANDLE;
}

bool VulkanInstance::Init()
{
	VkResult result;

	VkApplicationInfo appicationInfo{};
	appicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 26);

	VkInstanceCreateInfo instanceCI{};
	instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCI.pApplicationInfo = &appicationInfo;
	instanceCI.enabledLayerCount = (uint32_t)instanceLayers.size();
	instanceCI.enabledExtensionCount = (uint32_t)instanceExtensions.size();
	instanceCI.ppEnabledLayerNames = instanceLayers.data();
	instanceCI.ppEnabledExtensionNames = instanceExtensions.data();

	result = vkCreateInstance(&instanceCI, VK_NULL_HANDLE, &instance);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void VulkanInstance::AddInstanceLayer(const char * instanceLayerName)
{
	instanceLayers.push_back(instanceLayerName);
}

void VulkanInstance::AddInstanceExtension(const char * instanceExtensionName)
{
	instanceExtensions.push_back(instanceExtensionName);
}

VkInstance VulkanInstance::GetInstance()
{
	return instance;
}
