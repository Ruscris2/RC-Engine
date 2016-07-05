/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanSwapchain.cpp                                  |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanSwapchain.h"
#include "Settings.h"

extern Settings * gSettings;

VulkanSwapchain::VulkanSwapchain()
{
	swapChain = VK_NULL_HANDLE;
}

VulkanSwapchain::~VulkanSwapchain()
{
	swapChain = VK_NULL_HANDLE;
}

bool VulkanSwapchain::Init(VulkanDevice * vulkanDevice, VkImageView depthImageView, VulkanRenderpass * vulkanRenderpass)
{
	VkResult result;

	// Swap chain
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanDevice->GetGPU(), vulkanDevice->GetSurface(), &surfaceCapabilities);
	if (result != VK_SUCCESS)
		return false;

	uint32_t numPresentModes;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->GetGPU(), vulkanDevice->GetSurface(), &numPresentModes, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
		return false;

	VkPresentModeKHR * pPresentModes = new VkPresentModeKHR[numPresentModes];
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->GetGPU(), vulkanDevice->GetSurface(), &numPresentModes, pPresentModes);
	if (result != VK_SUCCESS)
		return false;

	VkExtent2D swapChainExtent;
	if (surfaceCapabilities.currentExtent.width == (uint32_t)-1)
	{
		swapChainExtent.width = gSettings->GetWindowWidth();
		swapChainExtent.height = gSettings->GetWindowHeight();
	}
	else
		swapChainExtent = surfaceCapabilities.currentExtent;

	VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < numPresentModes; i++)
	{
		if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		if ((swapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (pPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	// Use double-buffering
	uint32_t numSwapChainImages = 2;
	if (numSwapChainImages > surfaceCapabilities.maxImageCount)
		numSwapChainImages = surfaceCapabilities.maxImageCount;

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		preTransform = surfaceCapabilities.currentTransform;

	VkSwapchainCreateInfoKHR swapChainCI{};
	swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCI.surface = vulkanDevice->GetSurface();
	swapChainCI.minImageCount = numSwapChainImages;
	swapChainCI.imageFormat = vulkanDevice->GetFormat();
	swapChainCI.imageExtent.width = swapChainExtent.width;
	swapChainCI.imageExtent.height = swapChainExtent.height;
	swapChainCI.preTransform = preTransform;
	swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCI.imageArrayLayers = 1;
	swapChainCI.presentMode = swapChainPresentMode;
	swapChainCI.oldSwapchain = VK_NULL_HANDLE;
	swapChainCI.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapChainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCI.queueFamilyIndexCount = 0;
	swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCI.pQueueFamilyIndices = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(vulkanDevice->GetDevice(), &swapChainCI, VK_NULL_HANDLE, &swapChain);
	if (result != VK_SUCCESS)
		return false;

	uint32_t swapChainImageCount;
	result = vkGetSwapchainImagesKHR(vulkanDevice->GetDevice(), swapChain, &swapChainImageCount, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
		return false;

	VkImage * pSwapChainImages = new VkImage[swapChainImageCount];
	result = vkGetSwapchainImagesKHR(vulkanDevice->GetDevice(), swapChain, &swapChainImageCount, pSwapChainImages);

	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		SwapChainBuffer swapChainBuffer;

		VkImageViewCreateInfo viewCI{};
		viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCI.format = vulkanDevice->GetFormat();
		viewCI.components.r = VK_COMPONENT_SWIZZLE_R;
		viewCI.components.g = VK_COMPONENT_SWIZZLE_G;
		viewCI.components.b = VK_COMPONENT_SWIZZLE_B;
		viewCI.components.a = VK_COMPONENT_SWIZZLE_A;
		viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCI.subresourceRange.baseMipLevel = 0;
		viewCI.subresourceRange.levelCount = 1;
		viewCI.subresourceRange.baseArrayLayer = 0;
		viewCI.subresourceRange.layerCount = 1;
		viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCI.flags = 0;

		swapChainBuffer.image = pSwapChainImages[i];
		viewCI.image = swapChainBuffer.image;

		result = vkCreateImageView(vulkanDevice->GetDevice(), &viewCI, VK_NULL_HANDLE, &swapChainBuffer.view);
		swapChainBuffers.push_back(swapChainBuffer);
	}

	delete[] pSwapChainImages;
	delete[] pPresentModes;

	// Frame buffers
	VkImageView attachments[2];
	attachments[1] = depthImageView;

	VkFramebufferCreateInfo fbCI{};
	fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCI.renderPass = vulkanRenderpass->GetRenderpass();
	fbCI.attachmentCount = 2;
	fbCI.pAttachments = attachments;
	fbCI.width = gSettings->GetWindowWidth();
	fbCI.height = gSettings->GetWindowHeight();
	fbCI.layers = 1;
	
	frameBuffers = new VkFramebuffer[swapChainImageCount];

	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		attachments[0] = swapChainBuffers[i].view;
		result = vkCreateFramebuffer(vulkanDevice->GetDevice(), &fbCI, VK_NULL_HANDLE, &frameBuffers[i]);
		if (result != VK_SUCCESS)
			return false;
	}

	return true;
}

void VulkanSwapchain::Unload(VulkanDevice * vulkanDevice)
{
	for (uint32_t i = 0; i < swapChainBuffers.size(); i++)
	{
		vkDestroyFramebuffer(vulkanDevice->GetDevice(), frameBuffers[i], VK_NULL_HANDLE);
		vkDestroyImageView(vulkanDevice->GetDevice(), swapChainBuffers[i].view, VK_NULL_HANDLE);
	}
	vkDestroySwapchainKHR(vulkanDevice->GetDevice(), swapChain, VK_NULL_HANDLE);
}

void VulkanSwapchain::AcquireNextImage(VulkanDevice * vulkanDevice)
{
	VkSemaphoreCreateInfo semaphoreCI{};
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(vulkanDevice->GetDevice(), &semaphoreCI, VK_NULL_HANDLE, &presentCompleteSemaphore);
	vkCreateSemaphore(vulkanDevice->GetDevice(), &semaphoreCI, VK_NULL_HANDLE, &drawCompleteSemaphore);

	vkAcquireNextImageKHR(vulkanDevice->GetDevice(), swapChain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &currentBuffer);
}

void VulkanSwapchain::Present(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer)
{
	commandBuffer->Execute(vulkanDevice, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, presentCompleteSemaphore, drawCompleteSemaphore);

	VkPresentInfoKHR present;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = VK_NULL_HANDLE;
	present.swapchainCount = 1;
	present.pSwapchains = &swapChain;
	present.pImageIndices = &currentBuffer;
	present.waitSemaphoreCount = 1;
	present.pWaitSemaphores = &drawCompleteSemaphore;
	present.pResults = VK_NULL_HANDLE;
	vkQueuePresentKHR(vulkanDevice->GetQueue(), &present);

	vkDestroySemaphore(vulkanDevice->GetDevice(), presentCompleteSemaphore, VK_NULL_HANDLE);
	vkDestroySemaphore(vulkanDevice->GetDevice(), drawCompleteSemaphore, VK_NULL_HANDLE);
}

VkImage VulkanSwapchain::GetCurrentImage()
{
	return swapChainBuffers[currentBuffer].image;
}

VkFramebuffer VulkanSwapchain::GetCurrentFramebuffer()
{
	return frameBuffers[currentBuffer];
}
