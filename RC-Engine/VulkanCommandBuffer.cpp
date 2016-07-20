/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanCommandBuffer.cpp                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanCommandBuffer.h"
#include "LogManager.h"

extern LogManager * gLogManager;

VulkanCommandBuffer::VulkanCommandBuffer()
{
	commandBuffer = VK_NULL_HANDLE;
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	commandBuffer = VK_NULL_HANDLE;
}

bool VulkanCommandBuffer::Init(VulkanDevice * vulkanDevice, VulkanCommandPool * vulkanCommandPool, bool primary)
{
	VkResult result;
	VkCommandBufferAllocateInfo cmdBufferCI{};

	this->primary = primary;

	cmdBufferCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferCI.commandPool = vulkanCommandPool->GetCommandPool();
	cmdBufferCI.commandBufferCount = 1;
	
	if (primary)
		cmdBufferCI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	else
		cmdBufferCI.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	result = vkAllocateCommandBuffers(vulkanDevice->GetDevice(), &cmdBufferCI, &commandBuffer);
	if (result != VK_SUCCESS)
		return false;

	VkFenceCreateInfo fenceCI{};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	vkCreateFence(vulkanDevice->GetDevice(), &fenceCI, VK_NULL_HANDLE, &fence);

	return true;
}

void VulkanCommandBuffer::Unload(VulkanDevice * vulkanDevice, VulkanCommandPool * vulkanCommandPool)
{
	vkDestroyFence(vulkanDevice->GetDevice(), fence, VK_NULL_HANDLE);
	vkFreeCommandBuffers(vulkanDevice->GetDevice(), vulkanCommandPool->GetCommandPool(), 1, &commandBuffer);
}

void VulkanCommandBuffer::BeginRecording()
{
	if (primary)
	{
		VkCommandBufferBeginInfo beginCI{};
		beginCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginCI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginCI);
	}
	else
		gLogManager->AddMessage("WARNING: BeginRecording() called from a secondary command buffer!");
}

void VulkanCommandBuffer::BeginRecordingSecondary(VkRenderPass renderPass, VkFramebuffer framebuffer)
{
	if (!primary)
	{
		VkCommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.subpass = 0;
		inheritanceInfo.renderPass = renderPass;
		inheritanceInfo.framebuffer = framebuffer;

		VkCommandBufferBeginInfo beginCI{};
		beginCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginCI.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		beginCI.pInheritanceInfo = &inheritanceInfo;

		vkBeginCommandBuffer(commandBuffer, &beginCI);
	}
	else
		gLogManager->AddMessage("WARNING: BeginRecordingSecondary() called from a primary command buffer!");
}

void VulkanCommandBuffer::EndRecording()
{
	vkEndCommandBuffer(commandBuffer);
}

void VulkanCommandBuffer::Execute(VulkanDevice * device, VkPipelineStageFlags flags, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitFence)
{
	if (primary)
	{
		uint32_t waitSemaphoreCount = 1, signalSemaphoreCount = 1;

		if (waitSemaphore == NULL)
			waitSemaphoreCount = 0;

		if (signalSemaphore == NULL)
			signalSemaphoreCount = 0;

		VkSubmitInfo submitInfo{};
		submitInfo.pNext = VK_NULL_HANDLE;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = waitSemaphoreCount;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &flags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = signalSemaphoreCount;
		submitInfo.pSignalSemaphores = &signalSemaphore;

		if (waitFence)
		{
			vkQueueSubmit(device->GetQueue(), 1, &submitInfo, fence);

			vkWaitForFences(device->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
			vkResetFences(device->GetDevice(), 1, &fence);
		}
		else
			vkQueueSubmit(device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	}
	else
		gLogManager->AddMessage("WARNING: Used Execute on secondary command buffer!");
}

void VulkanCommandBuffer::ExecuteSecondary(VulkanCommandBuffer * primaryCmdBuffer)
{
	if (!primary)
		vkCmdExecuteCommands(primaryCmdBuffer->GetCommandBuffer(), 1, &commandBuffer);
	else
		gLogManager->AddMessage("WARNING: Used ExecuteSecondary on primary command buffer!");
}

VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer()
{
	return commandBuffer;
}
