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
	clear = NULL;
}

VulkanRenderpass::~VulkanRenderpass()
{
	delete[] clear;
	renderPass = VK_NULL_HANDLE;
}

bool VulkanRenderpass::Init(VulkanDevice * vulkanDevice, VulkanRenderpassCI * renderpassCI)
{
	VkResult result;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = renderpassCI->attachmentCount - 1;
	subpass.pColorAttachments = renderpassCI->attachmentRefs;
	subpass.pDepthStencilAttachment = renderpassCI->depthAttachmentRef;

	VkRenderPassCreateInfo vkRenderpassCI{};
	vkRenderpassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderpassCI.attachmentCount = renderpassCI->attachmentCount;
	vkRenderpassCI.pAttachments = renderpassCI->attachments;
	vkRenderpassCI.subpassCount = 1;
	vkRenderpassCI.pSubpasses = &subpass;
	vkRenderpassCI.dependencyCount = renderpassCI->dependenciesCount;
	vkRenderpassCI.pDependencies = renderpassCI->dependencies;

	result = vkCreateRenderPass(vulkanDevice->GetDevice(), &vkRenderpassCI, VK_NULL_HANDLE, &renderPass);
	if (result != VK_SUCCESS)
		return false;

	clear = new VkClearValue[renderpassCI->attachmentCount];
	clearCount = renderpassCI->attachmentCount;
	return true;
}

void VulkanRenderpass::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyRenderPass(vulkanDevice->GetDevice(), renderPass, VK_NULL_HANDLE);
}

void VulkanRenderpass::BeginRenderpass(VulkanCommandBuffer * commandBuffer, float r, float g, float b, float a, VkFramebuffer frame, VkSubpassContents contents)
{
	for (int i = 0; i < clearCount; i++)
	{
		clear[i].color.float32[0] = r;
		clear[i].color.float32[1] = g;
		clear[i].color.float32[2] = b;
		clear[i].color.float32[3] = a;
	}
	
	clear[clearCount - 1].depthStencil.depth = 1.0f;
	clear[clearCount - 1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rpBegin{};
	rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBegin.pNext = NULL;
	rpBegin.renderPass = renderPass;
	rpBegin.framebuffer = frame;
	rpBegin.renderArea.offset.x = 0;
	rpBegin.renderArea.offset.y = 0;
	rpBegin.renderArea.extent.width = gSettings->GetWindowWidth();
	rpBegin.renderArea.extent.height = gSettings->GetWindowHeight();
	rpBegin.clearValueCount = clearCount;
	rpBegin.pClearValues = clear;

	vkCmdBeginRenderPass(commandBuffer->GetCommandBuffer(), &rpBegin, contents);
}

void VulkanRenderpass::EndRenderpass(VulkanCommandBuffer * commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer->GetCommandBuffer());
}

VkRenderPass VulkanRenderpass::GetRenderpass()
{
	return renderPass;
}
