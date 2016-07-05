/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanRenderpass.cpp                                 |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanRenderpass.h"
#include "Settings.h"

extern Settings * gSettings;

VulkanRenderpass::VulkanRenderpass()
{
	renderPass = VK_NULL_HANDLE;
}

VulkanRenderpass::~VulkanRenderpass()
{
	renderPass = VK_NULL_HANDLE;
}

bool VulkanRenderpass::Init(VulkanDevice * vulkanDevice, VkAttachmentDescription * attachments, int attachmentCount)
{
	VkResult result;

	VkAttachmentReference colorRef{};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef{};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = &depthRef;

	VkRenderPassCreateInfo renderpassCI{};
	renderpassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCI.attachmentCount = attachmentCount;
	renderpassCI.pAttachments = attachments;
	renderpassCI.subpassCount = 1;
	renderpassCI.pSubpasses = &subpass;
	renderpassCI.dependencyCount = 0;
	renderpassCI.pDependencies = VK_NULL_HANDLE;

	result = vkCreateRenderPass(vulkanDevice->GetDevice(), &renderpassCI, VK_NULL_HANDLE, &renderPass);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void VulkanRenderpass::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyRenderPass(vulkanDevice->GetDevice(), renderPass, VK_NULL_HANDLE);
}

void VulkanRenderpass::BeginRenderpass(VulkanCommandBuffer * commandBuffer, float r, float g, float b, float a, VkFramebuffer frame)
{
	VkClearValue clear[2];
	clear[0].color.float32[0] = r;
	clear[0].color.float32[1] = g;
	clear[0].color.float32[2] = b;
	clear[0].color.float32[3] = a;
	clear[1].depthStencil.depth = 1.0f;
	clear[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rpBegin{};
	rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBegin.pNext = NULL;
	rpBegin.renderPass = renderPass;
	rpBegin.framebuffer = frame;
	rpBegin.renderArea.offset.x = 0;
	rpBegin.renderArea.offset.y = 0;
	rpBegin.renderArea.extent.width = gSettings->GetWindowWidth();
	rpBegin.renderArea.extent.height = gSettings->GetWindowHeight();
	rpBegin.clearValueCount = 2;
	rpBegin.pClearValues = clear;

	vkCmdBeginRenderPass(commandBuffer->GetCommandBuffer(), &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderpass::EndRenderpass(VulkanCommandBuffer * commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer->GetCommandBuffer());
}

VkRenderPass VulkanRenderpass::GetRenderpass()
{
	return renderPass;
}
