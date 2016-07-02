/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanCommandPool.cpp                                |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanCommandPool.h"

VulkanCommandPool::VulkanCommandPool()
{
	commandPool = VK_NULL_HANDLE;
}

VulkanCommandPool::~VulkanCommandPool()
{
	commandPool = VK_NULL_HANDLE;
}

bool VulkanCommandPool::Init(VulkanDevice * vulkanDevice)
{
	VkResult result;
	VkCommandPoolCreateInfo cmdPoolCI{};

	cmdPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCI.queueFamilyIndex = vulkanDevice->GetGraphicsQueueFamilyIndex();
	cmdPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	result = vkCreateCommandPool(vulkanDevice->GetDevice(), &cmdPoolCI, VK_NULL_HANDLE, &commandPool);
	if (result != VK_SUCCESS)
		return false;

	return true;
}

void VulkanCommandPool::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyCommandPool(vulkanDevice->GetDevice(), commandPool, VK_NULL_HANDLE);
}

VkCommandPool VulkanCommandPool::GetCommandPool()
{
	return commandPool;
}
