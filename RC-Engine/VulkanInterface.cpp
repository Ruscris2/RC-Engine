/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanInterface.cpp                                  |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <vector>

#include "VulkanInterface.h"
#include "LogManager.h"

extern LogManager * gLogManager;

bool minimalisticDebugInfo = true;

VulkanInterface::VulkanInterface()
{
	instance = VK_NULL_HANDLE;
	device = VK_NULL_HANDLE;
	gpu = VK_NULL_HANDLE;
}

VulkanInterface::~VulkanInterface()
{
#ifdef _DEBUG
		UnloadVulkanDebugMode();
#endif

	vkDestroyDevice(device, VK_NULL_HANDLE); device = VK_NULL_HANDLE;
	vkDestroyInstance(instance, VK_NULL_HANDLE); instance = VK_NULL_HANDLE;
}

bool VulkanInterface::Init()
{
	if (!InitInstance())
	{
		gLogManager->AddMessage("ERROR: Failed to init vulkan instance!");
		return false;
	}
#ifdef _DEBUG
		if (!InitVulkanDebugMode())
		{
			gLogManager->AddMessage("ERROR: Failed to init vulkan debug mode!");
			return false;
		}
#endif
	if (!InitDevice())
	{
		gLogManager->AddMessage("ERROR: Failed to init vulkan device!");
		return false;
	}
	return true;
}

bool VulkanInterface::InitInstance()
{
	VkResult result;

	// Instance layers and extensions
	std::vector<const char*> instanceLayers;
	std::vector<const char*> instanceExtensions;

#ifdef _DEBUG
	instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	// Instance creation
	VkApplicationInfo appicationInfo{};
	appicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 13);

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

bool VulkanInterface::InitDevice()
{
	VkResult result;

	// Device extensions
	std::vector<const char*> deviceExtensions;

	// GPU
	uint32_t numGPUs = 0;
	vkEnumeratePhysicalDevices(instance, &numGPUs, VK_NULL_HANDLE);
	if (numGPUs == 0)
		return false;

	std::vector<VkPhysicalDevice> pGPUs(numGPUs);
	vkEnumeratePhysicalDevices(instance, &numGPUs, pGPUs.data());
	gpu = pGPUs[0];

	vkGetPhysicalDeviceProperties(gpu, &gpuProperties);
	gLogManager->AddMessage("Rendering with: " + std::string(gpuProperties.deviceName));
	
	// Queue family
	uint32_t numQueueFamily = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &numQueueFamily, VK_NULL_HANDLE);
	if (numQueueFamily == 0)
		return false;

	std::vector<VkQueueFamilyProperties> pQueueFamilies(numQueueFamily);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &numQueueFamily, pQueueFamilies.data());

	bool found = false;
	for (uint32_t i = 0; i < numQueueFamily && !found; i++)
	{
		if (pQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			found = true;
			graphicsQueueFamilyIndex = i;
		}
	}

	if (!found)
	{
		gLogManager->AddMessage("ERROR: Couldn't find queue family index!");
		return false;
	}

	float pQueuePriorities[] = { 1.0f };
	VkDeviceQueueCreateInfo deviceQueueCI{};
	deviceQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCI.queueCount = 1;
	deviceQueueCI.queueFamilyIndex = graphicsQueueFamilyIndex;
	deviceQueueCI.pQueuePriorities = pQueuePriorities;

	// Device
	VkDeviceCreateInfo deviceCI{};
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &deviceQueueCI;
	deviceCI.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	deviceCI.ppEnabledExtensionNames = deviceExtensions.data();

	result = vkCreateDevice(gpu, &deviceCI, VK_NULL_HANDLE, &device);
	if (result != VK_SUCCESS)
	{
		gLogManager->AddMessage("ERROR: vkCreateDevice() failed!");
		return false;
	}

	return true;
}

#ifdef _DEBUG
	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT;

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObj, size_t location,
		int32_t msgCode, const char * layer_prefix, const char * msg, void * userData)
	{
		std::string outputMsg;
		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) outputMsg = "[INFO] ";
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) outputMsg = "[WARNING] ";
		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) outputMsg = "[PERF WARNING] ";
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) outputMsg = "[ERROR] ";
		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) outputMsg = "[DEBUG] ";

		outputMsg += msg;

		if (minimalisticDebugInfo == true)
		{
			if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT || flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
				gLogManager->AddMessage(outputMsg);
		}
		else
			gLogManager->AddMessage(outputMsg);

		return false;
	}

	bool VulkanInterface::InitVulkanDebugMode()
	{
		fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (fvkCreateDebugReportCallbackEXT == nullptr || fvkDestroyDebugReportCallbackEXT == nullptr)
		{
			gLogManager->AddMessage("ERROR: Couldn't fetch one or more debug functions!");
			return false;
		}

		VkDebugReportCallbackCreateInfoEXT debugCI{};
		debugCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		debugCI.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		debugCI.pfnCallback = VulkanDebugCallback;

		fvkCreateDebugReportCallbackEXT(instance, &debugCI, VK_NULL_HANDLE, &debugReport);
		return true;
	}

	void VulkanInterface::UnloadVulkanDebugMode()
	{
		fvkDestroyDebugReportCallbackEXT(instance, debugReport, VK_NULL_HANDLE);
		debugReport = VK_NULL_HANDLE;
	}
#endif