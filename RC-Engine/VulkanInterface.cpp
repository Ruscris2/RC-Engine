/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanInterface.cpp                                  |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "LogManager.h"
#include "Settings.h"
#include "StdInc.h"

extern LogManager * gLogManager;
extern Settings * gSettings;

bool minimalisticDebugInfo = true;

VulkanInterface::VulkanInterface()
{
	vulkanInstance = NULL;
	vulkanDevice = NULL;
	vulkanCommandPool = NULL;
	initCommandBuffer = NULL;
	renderCommandBuffer = NULL;
	vulkanSwapchain = NULL;
	vulkanShader = NULL;
	vulkanPipeline = NULL;
	model = NULL;
}

VulkanInterface::~VulkanInterface()
{
#ifdef _DEBUG
		UnloadVulkanDebugMode();
#endif
	vkFreeMemory(vulkanDevice->GetDevice(), depthImage.mem, VK_NULL_HANDLE); depthImage.mem = VK_NULL_HANDLE;
	vkDestroyImage(vulkanDevice->GetDevice(), depthImage.image, VK_NULL_HANDLE); depthImage.image = VK_NULL_HANDLE;
	vkDestroyImageView(vulkanDevice->GetDevice(), depthImage.view, VK_NULL_HANDLE); depthImage.view = VK_NULL_HANDLE;
	
	SAFE_DELETE(camera);
	SAFE_UNLOAD(model, vulkanDevice);
	SAFE_UNLOAD(vulkanPipeline, vulkanDevice);
	SAFE_UNLOAD(vulkanShader, vulkanDevice);
	SAFE_UNLOAD(vulkanSwapchain, vulkanDevice);
	SAFE_UNLOAD(renderCommandBuffer, vulkanDevice, vulkanCommandPool);
	SAFE_UNLOAD(initCommandBuffer, vulkanDevice, vulkanCommandPool);
	SAFE_UNLOAD(vulkanCommandPool, vulkanDevice);
	SAFE_UNLOAD(vulkanDevice, vulkanInstance);
	SAFE_DELETE(vulkanInstance);
}

bool VulkanInterface::Init(HWND hwnd)
{
	vulkanInstance = new VulkanInstance();
	#ifdef _DEBUG
	vulkanInstance->AddInstanceLayer("VK_LAYER_LUNARG_standard_validation");
	vulkanInstance->AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	#endif
	vulkanInstance->AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	vulkanInstance->AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);

	if (!vulkanInstance->Init())
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

	vulkanDevice = new VulkanDevice();
	vulkanDevice->AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	if (!vulkanDevice->Init(vulkanInstance, hwnd))
	{
		gLogManager->AddMessage("ERROR: Failed to init vulkan device!");
		return false;
	}

	vulkanCommandPool = new VulkanCommandPool();
	if (!vulkanCommandPool->Init(vulkanDevice))
	{
		gLogManager->AddMessage("ERROR: Failed to init command pool!");
		return false;
	}
	
	initCommandBuffer = new VulkanCommandBuffer();
	if (!initCommandBuffer->Init(vulkanDevice, vulkanCommandPool))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (initCommandBuffer)");
		return false;
	}

	renderCommandBuffer = new VulkanCommandBuffer();
	if (!renderCommandBuffer->Init(vulkanDevice, vulkanCommandPool))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (renderCommandBuffer)");
		return false;
	}

	if (!InitDepthBuffer())
	{
		gLogManager->AddMessage("ERROR: Failed to init depth buffer!");
		return false;
	}

	vulkanSwapchain = new VulkanSwapchain();
	if (!vulkanSwapchain->Init(vulkanDevice, depthImage.view, depthImage.format))
	{
		gLogManager->AddMessage("ERROR: Failed to create swapchain!");
		return false;
	}
	
	vulkanShader = new VulkanShader();
	if (!vulkanShader->Init(vulkanDevice))
	{
		gLogManager->AddMessage("ERROR: Failed to init shader!");
		return false;
	}
	
	vulkanPipeline = new VulkanPipeline();
	if (!vulkanPipeline->Init(vulkanDevice, vulkanShader, vulkanSwapchain))
	{
		gLogManager->AddMessage("ERROR: Failed to init pipeline!");
		return false;
	}

	model = new Model();
	if (!model->Init(vulkanDevice))
	{
		gLogManager->AddMessage("ERROR: Failed to init model!");
		return false;
	}
	
	camera = new Camera();
	camera->Init();
	return true;
}

void VulkanInterface::Render()
{
	camera->HandleInput();
	vulkanShader->Update(vulkanDevice, camera);

	renderCommandBuffer->BeginRecording();
	InitViewportAndScissors();

	vulkanSwapchain->AcquireNextImage(vulkanDevice);
	SetImageLayout(vulkanSwapchain->GetCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vulkanSwapchain->ClearImage(renderCommandBuffer, 0.0f, 0.0f, 0.0f, 1.0f);

	vulkanPipeline->SetActive(renderCommandBuffer);
	vulkanShader->SetActive(renderCommandBuffer);

	model->Render(renderCommandBuffer);
	vkCmdEndRenderPass(renderCommandBuffer->GetCommandBuffer());

	VkImageMemoryBarrier prePresentBarrier{};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.pNext = NULL;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    prePresentBarrier.subresourceRange.baseMipLevel = 0;
	prePresentBarrier.subresourceRange.levelCount = 1;
	prePresentBarrier.subresourceRange.baseArrayLayer = 0;
	prePresentBarrier.subresourceRange.layerCount = 1;
	prePresentBarrier.image = vulkanSwapchain->GetCurrentImage();
	vkCmdPipelineBarrier(renderCommandBuffer->GetCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &prePresentBarrier);

	renderCommandBuffer->EndRecording();
	vulkanSwapchain->Present(vulkanDevice, renderCommandBuffer);
}

bool VulkanInterface::InitDepthBuffer()
{
	VkResult result;
	VkImageCreateInfo imageCI{};
	depthImage.format = VK_FORMAT_D16_UNORM;

	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(vulkanDevice->GetGPU(), depthImage.format, &properties);
	if (properties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		imageCI.tiling = VK_IMAGE_TILING_LINEAR;
	else if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	else
	{
		gLogManager->AddMessage("ERROR: Depth format unsupported!");
		return false;
	}

	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = depthImage.format;
	imageCI.extent.width = gSettings->GetWindowWidth();
	imageCI.extent.height = gSettings->GetWindowHeight();
	imageCI.extent.depth = 1;
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCI.queueFamilyIndexCount = 0;
	imageCI.pQueueFamilyIndices = VK_NULL_HANDLE;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCI.flags = 0;

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	
	VkImageViewCreateInfo viewCI{};
	viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCI.image = VK_NULL_HANDLE;
	viewCI.format = depthImage.format;
	viewCI.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCI.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCI.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCI.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewCI.subresourceRange.baseMipLevel = 0;
	viewCI.subresourceRange.levelCount = 1;
	viewCI.subresourceRange.baseArrayLayer = 0;
	viewCI.subresourceRange.layerCount = 1;
	viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCI.flags = 0;

	VkMemoryRequirements memReq;
	result = vkCreateImage(vulkanDevice->GetDevice(), &imageCI, VK_NULL_HANDLE, &depthImage.image);
	if (result != VK_SUCCESS)
		return false;

	vkGetImageMemoryRequirements(vulkanDevice->GetDevice(), depthImage.image, &memReq);
	memAlloc.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, 0, &memAlloc.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &memAlloc, VK_NULL_HANDLE, &depthImage.mem);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindImageMemory(vulkanDevice->GetDevice(), depthImage.image, depthImage.mem, 0);
	if (result != VK_SUCCESS)
		return false;

	SetImageLayout(depthImage.image, viewCI.subresourceRange.aspectMask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	
	viewCI.image = depthImage.image;
	result = vkCreateImageView(vulkanDevice->GetDevice(), &viewCI, VK_NULL_HANDLE, &depthImage.view);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void VulkanInterface::InitViewportAndScissors()
{
	viewport.width = (float)gSettings->GetWindowWidth();
	viewport.height = (float)gSettings->GetWindowHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(renderCommandBuffer->GetCommandBuffer(), 0, 1, &viewport);

	scissor.extent.width = (uint32_t)gSettings->GetWindowWidth();
	scissor.extent.height = (uint32_t)gSettings->GetWindowHeight();
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(renderCommandBuffer->GetCommandBuffer(), 0, 1, &scissor);
}

void VulkanInterface::SetImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
	initCommandBuffer->BeginRecording();

	VkImageMemoryBarrier imageMemBarrier{};
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.oldLayout = oldImageLayout;
	imageMemBarrier.newLayout = newImageLayout;
	imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.image = image;
	imageMemBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemBarrier.subresourceRange.baseMipLevel = 0;
	imageMemBarrier.subresourceRange.levelCount = 1;
	imageMemBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemBarrier.subresourceRange.layerCount = 1;

	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		imageMemBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		imageMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		imageMemBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		imageMemBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(initCommandBuffer->GetCommandBuffer(), srcStages, destStages, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &imageMemBarrier);

	initCommandBuffer->EndRecording();
	initCommandBuffer->Execute(vulkanDevice, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, NULL, NULL);
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
		fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance->GetInstance(), "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanInstance->GetInstance(), "vkDestroyDebugReportCallbackEXT");
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

		fvkCreateDebugReportCallbackEXT(vulkanInstance->GetInstance(), &debugCI, VK_NULL_HANDLE, &debugReport);
		return true;
	}

	void VulkanInterface::UnloadVulkanDebugMode()
	{
		fvkDestroyDebugReportCallbackEXT(vulkanInstance->GetInstance(), debugReport, VK_NULL_HANDLE);
		debugReport = VK_NULL_HANDLE;
	}
#endif
