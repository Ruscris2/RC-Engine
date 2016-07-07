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

bool VulkanRenderpass::Init(VulkanDevice * vulkanDevice, VkAttachmentDescription * attachments, int attachmentCount, VkAttachmentReference * attachmentRefs, int attachRefCount, int depthRefIndex)
{
	VkResult result;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = attachmentCount - 1;
	subpass.pColorAttachments = attachmentRefs;
	subpass.pDepthStencilAttachment = &attachmentRefs[depthRefIndex];

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

	clear = new VkClearValue[attachmentCount];
	depthClearIndex = depthRefIndex;
	clearCount = attachmentCount;
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
	
	clear[depthClearIndex].depthStencil.depth = 1.0f;
	clear[depthClearIndex].depthStencil.stencil = 0;

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
