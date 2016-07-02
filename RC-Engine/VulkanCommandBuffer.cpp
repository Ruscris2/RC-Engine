/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanCommandBuffer.cpp                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanCommandBuffer.h"

VulkanCommandBuffer::VulkanCommandBuffer()
{
	commandBuffer = VK_NULL_HANDLE;
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	commandBuffer = VK_NULL_HANDLE;
}

bool VulkanCommandBuffer::Init(VulkanDevice * vulkanDevice, VulkanCommandPool * vulkanCommandPool)
{
	VkResult result;
	VkCommandBufferAllocateInfo cmdBufferCI{};

	cmdBufferCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferCI.commandPool = vulkanCommandPool->GetCommandPool();
	cmdBufferCI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferCI.commandBufferCount = 1;

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
	VkCommandBufferBeginInfo beginCI{};
	beginCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginCI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginCI);
}

void VulkanCommandBuffer::EndRecording()
{
	vkEndCommandBuffer(commandBuffer);
}

void VulkanCommandBuffer::Execute(VulkanDevice * device, VkPipelineStageFlags flags, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore)
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

	vkQueueSubmit(device->GetQueue(), 1, &submitInfo, fence);

	vkWaitForFences(device->GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device->GetDevice(), 1, &fence);

	vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer()
{
	return commandBuffer;
}
