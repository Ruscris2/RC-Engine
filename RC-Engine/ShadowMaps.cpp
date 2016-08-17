/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: ShadowMaps.cpp                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "ShadowMaps.h"
#include "LogManager.h"
#include "StdInc.h"

extern LogManager * gLogManager;

ShadowMaps::ShadowMaps()
{
	depthAttachment = NULL;
	renderpass = NULL;
	camera = NULL;
}

bool ShadowMaps::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer)
{
	VkResult result;

	mapWidth = mapHeight = 1024;

	// Create framebuffer attachments
	depthAttachment = new FrameBufferAttachment();
	if (!depthAttachment->Create(vulkan->GetVulkanDevice(), vulkan->GetDepthAttachment()->GetFormat(),
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, cmdBuffer, mapWidth, mapHeight))
	{
		gLogManager->AddMessage("ERROR: Failed to create depth framebuffer attachment!");
		return false;
	}

	// Create the renderpass
	VkAttachmentDescription attachmentDesc{};
	VkAttachmentReference attachmentRef;

	attachmentDesc.format = vulkan->GetDepthAttachment()->GetFormat();
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.flags = 0;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	attachmentRef.attachment = 0;
	attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VulkanRenderpassCI renderpassCI;
	renderpassCI.attachments = &attachmentDesc;
	renderpassCI.attachmentCount = 1;
	renderpassCI.attachmentRefs = VK_NULL_HANDLE;
	renderpassCI.depthAttachmentRef = &attachmentRef;
	renderpassCI.dependencies = VK_NULL_HANDLE;
	renderpassCI.dependenciesCount = 0;

	renderpass = new VulkanRenderpass();
	if (!renderpass->Init(vulkan->GetVulkanDevice(), &renderpassCI))
	{
		gLogManager->AddMessage("ERROR: Failed to create shadow map renderpass!");
		return false;
	}

	// Create the framebuffer
	VkFramebufferCreateInfo fbCI{};
	fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCI.renderPass = renderpass->GetRenderpass();
	fbCI.pAttachments = depthAttachment->GetImageView();
	fbCI.attachmentCount = 1;
	fbCI.width = mapWidth;
	fbCI.height = mapHeight;
	fbCI.layers = 1;

	result = vkCreateFramebuffer(vulkan->GetVulkanDevice()->GetDevice(), &fbCI, VK_NULL_HANDLE, &framebuffer);
	if (result != VK_SUCCESS)
		return false;

	// Create the sampler
	VkSamplerCreateInfo samplerCI{};
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 1.0f;
	samplerCI.maxAnisotropy = 0.0f;
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	result = vkCreateSampler(vulkan->GetVulkanDevice()->GetDevice(), &samplerCI, VK_NULL_HANDLE, &sampler);
	if (result != VK_SUCCESS)
		return false;

	// Create camera
	camera = new Camera();
	camera->SetCameraState(CAMERA_STATE_LOOK_AT);
	camera->SetPosition(0.0f, 6.0f, -5.0f);
	camera->SetLookAt(0.0f, 0.0f, 0.0f);

	
	orthoMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -60.0f, 60.0f);
	return true;
}

void ShadowMaps::Unload(VulkanInterface * vulkan)
{
	vkDestroySampler(vulkan->GetVulkanDevice()->GetDevice(), sampler, VK_NULL_HANDLE);
	vkDestroyFramebuffer(vulkan->GetVulkanDevice()->GetDevice(), framebuffer, VK_NULL_HANDLE);
	SAFE_UNLOAD(renderpass, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(depthAttachment, vulkan->GetVulkanDevice());
}

void ShadowMaps::BeginShadowPass(VulkanCommandBuffer * commandBuffer)
{
	commandBuffer->BeginRecording();

	renderpass->BeginRenderpass(commandBuffer, 0.0f, 0.0f, 0.0f, 0.0f, framebuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
		mapWidth, mapHeight);
}

void ShadowMaps::EndShadowPass(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer)
{
	renderpass->EndRenderpass(commandBuffer);

	commandBuffer->EndRecording();

	commandBuffer->Execute(vulkanDevice, NULL, NULL, NULL, true);
}

void ShadowMaps::SetDepthBias(VulkanCommandBuffer * cmdBuffer)
{
	vkCmdSetDepthBias(cmdBuffer->GetCommandBuffer(), 1.25f, 0.0f, 1.75f);
}

VulkanRenderpass * ShadowMaps::GetShadowRenderpass()
{
	return renderpass;
}

VkFramebuffer ShadowMaps::GetFramebuffer()
{
	return framebuffer;
}

VkImageView * ShadowMaps::GetImageView()
{
	return depthAttachment->GetImageView();
}

Camera * ShadowMaps::GetCamera()
{
	return camera;
}

glm::mat4 ShadowMaps::GetOrthoMatrix()
{
	return orthoMatrix;
}

VkSampler ShadowMaps::GetSampler()
{
	return sampler;
}

uint32_t ShadowMaps::GetMapWidth()
{
	return mapWidth;
}

uint32_t ShadowMaps::GetMapHeight()
{
	return mapHeight;
}
