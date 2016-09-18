/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanDevice.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanDevice.h"
#include "LogManager.h"

extern LogManager * gLogManager;

VulkanDevice::VulkanDevice()
{
	device = VK_NULL_HANDLE;
	surface = VK_NULL_HANDLE;
}

VulkanDevice::~VulkanDevice()
{
	device = VK_NULL_HANDLE;
	surface = VK_NULL_HANDLE;
}

bool VulkanDevice::Init(VulkanInstance * vulkanInstance, HWND hwnd)
{
	VkResult result;

	// GPU
	uint32_t numGPUs = 0;
	vkEnumeratePhysicalDevices(vulkanInstance->GetInstance(), &numGPUs, VK_NULL_HANDLE);
	if (numGPUs == 0)
	{
		gLogManager->AddMessage("ERROR: No GPUs found!");
		return false;
	}

	std::vector<VkPhysicalDevice> pGPUs(numGPUs);
	vkEnumeratePhysicalDevices(vulkanInstance->GetInstance(), &numGPUs, pGPUs.data());
	gpu = pGPUs[0];

	vkGetPhysicalDeviceProperties(gpu, &gpuProperties);
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);
	gLogManager->AddMessage("Rendering with: " + std::string(gpuProperties.deviceName));

	// Queue family
	uint32_t numQueueFamily = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &numQueueFamily, VK_NULL_HANDLE);
	if (numQueueFamily == 0)
	{
		gLogManager->AddMessage("ERROR: No Queue Families were found!");
		return false;
	}

	queueFamiliyProperties.resize(numQueueFamily);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &numQueueFamily, queueFamiliyProperties.data());

	// Surface
	VkWin32SurfaceCreateInfoKHR win32SurfaceCI{};
	win32SurfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCI.hinstance = GetModuleHandle(NULL);
	win32SurfaceCI.hwnd = hwnd;

	result = vkCreateWin32SurfaceKHR(vulkanInstance->GetInstance(), &win32SurfaceCI, VK_NULL_HANDLE, &surface);
	if (result != VK_SUCCESS)
	{
		gLogManager->AddMessage("ERROR: Couldn't create Win32 Surface!");
		return false;
	}

	VkBool32 * supportsPresent = new VkBool32[queueFamiliyProperties.size()];
	for (uint32_t i = 0; i < queueFamiliyProperties.size(); i++)
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supportsPresent[i]);

	graphicsQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueFamiliyProperties.size(); i++)
	{
		if ((queueFamiliyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueFamilyIndex = i;
				break;
			}
		}
	}

	delete[] supportsPresent;

	if (graphicsQueueFamilyIndex == UINT32_MAX)
	{
		gLogManager->AddMessage("ERROR: Couldn't find a graphics queue family index!");
		return false;
	}

	uint32_t numFormats;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &numFormats, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
	{
		gLogManager->AddMessage("ERROR: Couldn't get surface formats!");
		return false;
	}

	VkSurfaceFormatKHR * pSurfaceFormats = new VkSurfaceFormatKHR[numFormats];
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &numFormats, pSurfaceFormats);

	if (numFormats == 1 && pSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		format = VK_FORMAT_B8G8R8A8_UNORM;
	else
		format = pSurfaceFormats[0].format;

	// Device queue

	float pQueuePriorities[] = { 1.0f };
	VkDeviceQueueCreateInfo deviceQueueCI{};
	deviceQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCI.queueCount = 1;
	deviceQueueCI.queueFamilyIndex = graphicsQueueFamilyIndex;
	deviceQueueCI.pQueuePriorities = pQueuePriorities;

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.shaderClipDistance = VK_TRUE;
	deviceFeatures.shaderCullDistance = VK_TRUE;
	deviceFeatures.geometryShader = VK_TRUE;
	deviceFeatures.shaderTessellationAndGeometryPointSize = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	// Device
	VkDeviceCreateInfo deviceCI{};
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &deviceQueueCI;
	deviceCI.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	deviceCI.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCI.pEnabledFeatures = &deviceFeatures;

	result = vkCreateDevice(gpu, &deviceCI, VK_NULL_HANDLE, &device);
	if (result != VK_SUCCESS)
	{
		gLogManager->AddMessage("ERROR: vkCreateDevice() failed!");
		return false;
	}

	vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &deviceQueue);
	return true;
}

void VulkanDevice::Unload(VulkanInstance * vulkanInstance)
{
	vkDestroyDevice(device, VK_NULL_HANDLE);
	vkDestroySurfaceKHR(vulkanInstance->GetInstance(), surface, VK_NULL_HANDLE);
}

void VulkanDevice::AddDeviceExtension(const char * deviceExtensionName)
{
	deviceExtensions.push_back(deviceExtensionName);
}

VkDevice VulkanDevice::GetDevice()
{
	return device;
}

VkPhysicalDevice VulkanDevice::GetGPU()
{
	return gpu;
}

VkQueue VulkanDevice::GetQueue()
{
	return deviceQueue;
}

uint32_t VulkanDevice::GetGraphicsQueueFamilyIndex()
{
	return graphicsQueueFamilyIndex;
}

VkSurfaceKHR VulkanDevice::GetSurface()
{
	return surface;
}

VkFormat VulkanDevice::GetFormat()
{
	return format;
}

VkPhysicalDeviceProperties VulkanDevice::GetGPUProperties()
{
	return gpuProperties;
}

bool VulkanDevice::MemoryTypeFromProperties(uint32_t typeBits, VkFlags reqMask, uint32_t * typeIndex)
{
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & reqMask) == reqMask)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}