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
#include "VulkanTools.h"

extern LogManager * gLogManager;
extern Settings * gSettings;

bool minimalisticDebugInfo = true;

VulkanInterface::VulkanInterface()
{
	vulkanInstance = NULL;
	vulkanDevice = NULL;
	vulkanCommandPool = NULL;
	initCommandBuffer = NULL;
	vulkanSwapchain = NULL;
	mainRenderPass = NULL;
	deferredRenderPass = NULL;

	positionAtt = NULL;
	normalAtt = NULL;
	albedoAtt = NULL;
	depthAtt = NULL;
}

VulkanInterface::~VulkanInterface()
{
#ifdef _DEBUG
		UnloadVulkanDebugMode();
#endif
	vkDestroySemaphore(vulkanDevice->GetDevice(), drawCompleteSemaphore, VK_NULL_HANDLE);
	vkDestroySemaphore(vulkanDevice->GetDevice(), presentCompleteSemaphore, VK_NULL_HANDLE);

	vkDestroyFramebuffer(vulkanDevice->GetDevice(), deferredFramebuffer, VK_NULL_HANDLE);
	SAFE_UNLOAD(deferredRenderPass, vulkanDevice);
	SAFE_UNLOAD(depthAtt, vulkanDevice);
	SAFE_UNLOAD(albedoAtt, vulkanDevice);
	SAFE_UNLOAD(normalAtt, vulkanDevice);
	SAFE_UNLOAD(positionAtt, vulkanDevice);
	
	vkDestroySampler(vulkanDevice->GetDevice(), colorSampler, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), depthImage.mem, VK_NULL_HANDLE); depthImage.mem = VK_NULL_HANDLE;
	vkDestroyImage(vulkanDevice->GetDevice(), depthImage.image, VK_NULL_HANDLE); depthImage.image = VK_NULL_HANDLE;
	vkDestroyImageView(vulkanDevice->GetDevice(), depthImage.view, VK_NULL_HANDLE); depthImage.view = VK_NULL_HANDLE;
	
	SAFE_UNLOAD(vulkanSwapchain, vulkanDevice);
	SAFE_UNLOAD(mainRenderPass, vulkanDevice);
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
	if (!initCommandBuffer->Init(vulkanDevice, vulkanCommandPool, true))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (initCommandBuffer)");
		return false;
	}

	if (!InitDepthBuffer())
	{
		gLogManager->AddMessage("ERROR: Failed to init depth buffer!");
		return false;
	}

	VkAttachmentDescription attachmentDesc[2];
	attachmentDesc[0].format = vulkanDevice->GetFormat();
	attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc[0].flags = 0;

	attachmentDesc[1].format = depthImage.format;
	attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDesc[1].flags = 0;

	VkAttachmentReference attachmentRefs[2];
	attachmentRefs[0].attachment = 0;
	attachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachmentRefs[1].attachment = 1;
	attachmentRefs[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	mainRenderPass = new VulkanRenderpass();
	if (!mainRenderPass->Init(vulkanDevice, attachmentDesc, 2, attachmentRefs, 2, 1))
	{
		gLogManager->AddMessage("ERROR: Failed to init main render pass!");
		return false;
	}

	vulkanSwapchain = new VulkanSwapchain();
	if (!vulkanSwapchain->Init(vulkanDevice, depthImage.view, mainRenderPass))
	{
		gLogManager->AddMessage("ERROR: Failed to create swapchain!");
		return false;
	}

	if (!InitDeferredFramebuffer())
	{
		gLogManager->AddMessage("ERROR: Failed to init deferred framebuffer!");
		return false;
	}

	if (!InitColorSampler())
	{
		gLogManager->AddMessage("ERROR: Failed to init color sampler!");
		return false;
	}

	float fov = glm::radians(45.0f);
	float aspectRatio = (float)gSettings->GetWindowWidth() / gSettings->GetWindowHeight();
	projectionMatrix = glm::perspective(fov, aspectRatio, 0.1f, 100.0f);
	orthoMatrix = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

	VkSemaphoreCreateInfo semaphoreCI{};
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(vulkanDevice->GetDevice(), &semaphoreCI, VK_NULL_HANDLE, &presentCompleteSemaphore);
	vkCreateSemaphore(vulkanDevice->GetDevice(), &semaphoreCI, VK_NULL_HANDLE, &drawCompleteSemaphore);

	return true;
}

void VulkanInterface::BeginScene3D(VulkanCommandBuffer * commandBuffer)
{
	commandBuffer->BeginRecording();
	InitViewportAndScissors(commandBuffer);

	std::vector<FrameBufferAttachment*> attachments;
	attachments.push_back(positionAtt);
	attachments.push_back(normalAtt);
	attachments.push_back(albedoAtt);

	for (unsigned int i = 0; i < attachments.size(); i++)
		VulkanTools::SetImageLayout(attachments[i]->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			NULL, commandBuffer, vulkanDevice, false);
	
	deferredRenderPass->BeginRenderpass(commandBuffer, 0.0f, 0.0f, 0.0f, 1.0f, deferredFramebuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void VulkanInterface::EndScene3D(VulkanCommandBuffer * commandBuffer)
{
	deferredRenderPass->EndRenderpass(commandBuffer);

	std::vector<FrameBufferAttachment*> attachments;
	attachments.push_back(positionAtt);
	attachments.push_back(normalAtt);
	attachments.push_back(albedoAtt);

	for (unsigned int i = 0; i < attachments.size(); i++)
		VulkanTools::SetImageLayout(attachments[i]->GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			NULL, commandBuffer, vulkanDevice, false);
	
	commandBuffer->EndRecording();

	commandBuffer->Execute(vulkanDevice, NULL, NULL, NULL);
}

void VulkanInterface::BeginScene2D(VulkanCommandBuffer * commandBuffer, VulkanPipeline * pipeline)
{
	commandBuffer->BeginRecording();
	InitViewportAndScissors(commandBuffer);

	vulkanSwapchain->AcquireNextImage(vulkanDevice, presentCompleteSemaphore);
	VulkanTools::SetImageLayout(vulkanSwapchain->GetCurrentImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, NULL, initCommandBuffer, vulkanDevice, true);
	mainRenderPass->BeginRenderpass(commandBuffer, 0.0f, 0.0f, 0.0f, 1.0f, vulkanSwapchain->GetCurrentFramebuffer(), VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanInterface::EndScene2D(VulkanCommandBuffer * commandBuffer)
{
	mainRenderPass->EndRenderpass(commandBuffer);

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
	vkCmdPipelineBarrier(commandBuffer->GetCommandBuffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &prePresentBarrier);

	commandBuffer->EndRecording();

	commandBuffer->Execute(vulkanDevice, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, presentCompleteSemaphore, drawCompleteSemaphore);
	vulkanSwapchain->Present(vulkanDevice, commandBuffer, drawCompleteSemaphore);
}

VulkanCommandPool * VulkanInterface::GetVulkanCommandPool()
{
	return vulkanCommandPool;
}

VulkanDevice * VulkanInterface::GetVulkanDevice()
{
	return vulkanDevice;
}

VulkanRenderpass * VulkanInterface::GetMainRenderpass()
{
	return mainRenderPass;
}

VulkanRenderpass * VulkanInterface::GetDeferredRenderpass()
{
	return deferredRenderPass;
}

glm::mat4 VulkanInterface::GetProjectionMatrix()
{
	return projectionMatrix;
}

glm::mat4 VulkanInterface::GetOrthoMatrix()
{
	return orthoMatrix;
}

VkSampler VulkanInterface::GetColorSampler()
{
	return colorSampler;
}

FrameBufferAttachment * VulkanInterface::GetPositionAttachment()
{
	return positionAtt;
}

FrameBufferAttachment * VulkanInterface::GetNormalAttachment()
{
	return normalAtt;
}

FrameBufferAttachment * VulkanInterface::GetAlbedoAttachment()
{
	return albedoAtt;
}

VkFramebuffer VulkanInterface::GetDeferredFramebuffer()
{
	return deferredFramebuffer;
}

bool VulkanInterface::InitDepthBuffer()
{
	VkResult result;
	VkImageCreateInfo imageCI{};

	std::vector<VkFormat> depthFormats;
	depthFormats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
	depthFormats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
	depthFormats.push_back(VK_FORMAT_D16_UNORM_S8_UINT);
	
	VkFormatProperties properties;
	for (unsigned int i = 0; i < depthFormats.size(); i++)
	{
		vkGetPhysicalDeviceFormatProperties(vulkanDevice->GetGPU(), depthFormats[i], &properties);
		if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthImage.format = depthFormats[i];
			break;
		}
	}

	vkGetPhysicalDeviceFormatProperties(vulkanDevice->GetGPU(), depthImage.format, &properties);
	if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
	{
		gLogManager->AddMessage("ERROR: Couldn't find a depth image format!");
		return false;
	}

	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = depthImage.format;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
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
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
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

	VulkanTools::SetImageLayout(depthImage.image, viewCI.subresourceRange.aspectMask, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, NULL, initCommandBuffer, vulkanDevice, true);
	
	viewCI.image = depthImage.image;
	result = vkCreateImageView(vulkanDevice->GetDevice(), &viewCI, VK_NULL_HANDLE, &depthImage.view);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

bool VulkanInterface::InitDeferredFramebuffer()
{
	VkResult result;

	positionAtt = new FrameBufferAttachment();
	if (!positionAtt->Create(vulkanDevice, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to create position framebuffer attachment!");
		return false;
	}

	normalAtt = new FrameBufferAttachment();
	if (!normalAtt->Create(vulkanDevice, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to create normal framebuffer attachment!");
		return false;
	}

	albedoAtt = new FrameBufferAttachment();
	if (!albedoAtt->Create(vulkanDevice, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to create albedo framebuffer attachment!");
		return false;
	}

	depthAtt = new FrameBufferAttachment();
	if (!depthAtt->Create(vulkanDevice, depthImage.format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to create depth framebuffer attachment!");
		return false;
	}

	std::vector<VkAttachmentDescription> attachmentDescs;
	std::vector<VkAttachmentReference> attachmentRefs;
	attachmentDescs.resize(4);
	attachmentRefs.resize(4);

	for (unsigned int i = 0; i < attachmentDescs.size(); i++)
	{
		attachmentDescs[i] = {};
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[i].flags = 0;
		attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	// Overwrite layout for depth
	attachmentDescs[3].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachmentDescs[0].format = positionAtt->GetFormat();
	attachmentDescs[1].format = normalAtt->GetFormat();
	attachmentDescs[2].format = albedoAtt->GetFormat();
	attachmentDescs[3].format = depthAtt->GetFormat();

	for (unsigned int i = 0; i < attachmentRefs.size(); i++)
	{
		attachmentRefs[i].attachment = i;
		attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	// Overwrite reference for depth
	attachmentRefs[3].attachment = 3;
	attachmentRefs[3].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	deferredRenderPass = new VulkanRenderpass();
	if (!deferredRenderPass->Init(vulkanDevice, (VkAttachmentDescription*)attachmentDescs.data(), 4, (VkAttachmentReference*)attachmentRefs.data(), 4, 3))
	{
		gLogManager->AddMessage("ERROR: Failed to init deferred renderpass!");
		return false;
	}

	std::vector<VkImageView> viewAttachments;
	viewAttachments.resize(4);

	viewAttachments[0] = positionAtt->GetImageView();
	viewAttachments[1] = normalAtt->GetImageView();
	viewAttachments[2] = albedoAtt->GetImageView();
	viewAttachments[3] = depthAtt->GetImageView();

	VkFramebufferCreateInfo fbCI{};
	fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCI.renderPass = deferredRenderPass->GetRenderpass();
	fbCI.pAttachments = viewAttachments.data();
	fbCI.attachmentCount = (uint32_t)viewAttachments.size();
	fbCI.width = gSettings->GetWindowWidth();
	fbCI.height = gSettings->GetWindowHeight();
	fbCI.layers = 1;

	result = vkCreateFramebuffer(vulkanDevice->GetDevice(), &fbCI, VK_NULL_HANDLE, &deferredFramebuffer);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

bool VulkanInterface::InitColorSampler()
{
	VkResult result;

	VkSamplerCreateInfo samplerCI{};
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.compareOp = VK_COMPARE_OP_NEVER;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 0.0f;
	samplerCI.maxAnisotropy = vulkanDevice->GetGPUProperties().limits.maxSamplerAnisotropy;
	samplerCI.anisotropyEnable = VK_TRUE;
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	result = vkCreateSampler(vulkanDevice->GetDevice(), &samplerCI, VK_NULL_HANDLE, &colorSampler);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void VulkanInterface::InitViewportAndScissors(VulkanCommandBuffer * commandBuffer)
{
	viewport.width = (float)gSettings->GetWindowWidth();
	viewport.height = (float)gSettings->GetWindowHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0;
	viewport.y = 0;
	vkCmdSetViewport(commandBuffer->GetCommandBuffer(), 0, 1, &viewport);

	scissor.extent.width = (uint32_t)gSettings->GetWindowWidth();
	scissor.extent.height = (uint32_t)gSettings->GetWindowHeight();
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(commandBuffer->GetCommandBuffer(), 0, 1, &scissor);
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
		if (minimalisticDebugInfo == true)
		{
			debugCI.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
		}
		else
		{
			debugCI.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		}
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
