/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Canvas.cpp                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Canvas.h"
#include "StdInc.h"
#include "Settings.h"

extern Settings * gSettings;

Canvas::Canvas()
{
	vertexBuffer = VK_NULL_HANDLE;
	vsUniformBuffer = VK_NULL_HANDLE;

	posX = posY = 0.0f;
	width = height = 0.25f;
}

Canvas::~Canvas()
{
	delete[] vertexData;

	vsUniformBuffer = VK_NULL_HANDLE;
	vertexBuffer = VK_NULL_HANDLE;
}

bool Canvas::Init(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	VkResult result;

	vertexCount = 6;
	vertexData = new Vertex[vertexCount];
	UpdateVertexData();

	uint8_t *pData;
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	// Vertex buffer
	VkBufferCreateInfo vertexBufferCI{};
	vertexBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferCI.size = sizeof(Vertex) * vertexCount;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vertexBufferCI, VK_NULL_HANDLE, &vertexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), vertexBuffer, &vertexBufferMemReq);

	allocInfo.allocationSize = vertexBufferMemReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(vertexBufferMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &vertexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), vertexMemory, 0, vertexBufferMemReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, vertexData, sizeof(Vertex) * vertexCount);

	vkUnmapMemory(vulkanDevice->GetDevice(), vertexMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), vertexBuffer, vertexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	// Uniform inits
	vertexUniformBuffer.MVP = glm::mat4();

	// Vertex shader Uniform buffer
	VkBufferCreateInfo vsBufferCI{};
	vsBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vsBufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vsBufferCI.size = sizeof(vertexUniformBuffer);
	vsBufferCI.queueFamilyIndexCount = 0;
	vsBufferCI.pQueueFamilyIndices = VK_NULL_HANDLE;
	vsBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vsBufferCI, VK_NULL_HANDLE, &vsUniformBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), vsUniformBuffer, &vsMemReq);

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = vsMemReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(vsMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &vsUniformMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	vkUnmapMemory(vulkanDevice->GetDevice(), vsUniformMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), vsUniformBuffer, vsUniformMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	vsUniformBufferInfo.buffer = vsUniformBuffer;
	vsUniformBufferInfo.offset = 0;
	vsUniformBufferInfo.range = sizeof(vertexUniformBuffer);

	// Init draw command buffers
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
		if (!cmdBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), false))
			return false;

		drawCmdBuffers.push_back(cmdBuffer);
	}

	updateVertexBuffer = false;
	return true;
}

void Canvas::Unload(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();

	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
		SAFE_UNLOAD(drawCmdBuffers[i], vulkanDevice, vulkan->GetVulkanCommandPool());

	vkFreeMemory(vulkanDevice->GetDevice(), vsUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vsUniformBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), vertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vertexBuffer, VK_NULL_HANDLE);
}

void Canvas::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
	glm::mat4 orthoMatrix, VkImageView * imageView, int frameBufferId)
{
	uint8_t *pData;

	// Update vertex buffer if needed
	if (updateVertexBuffer)
	{
		UpdateVertexData();
		vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), vertexMemory, 0, vertexBufferMemReq.size, 0, (void**)&pData);
		memcpy(pData, vertexData, sizeof(Vertex) * vertexCount);
		vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), vertexMemory);
		updateVertexBuffer = false;
	}

	UpdateDescriptorSet(vulkan, vulkanPipeline, imageView);

	// Update vertex uniform buffer
	vertexUniformBuffer.MVP = orthoMatrix;

	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory);

	// Draw
	drawCmdBuffers[frameBufferId]->BeginRecordingSecondary(vulkan->GetForwardRenderpass()->GetRenderpass(), vulkan->GetVulkanSwapchain()->GetFramebuffer((int)frameBufferId));
	vulkan->InitViewportAndScissors(drawCmdBuffers[frameBufferId], (float)gSettings->GetWindowWidth(), (float)gSettings->GetWindowHeight(),
		(uint32_t)gSettings->GetWindowWidth(), (uint32_t)gSettings->GetWindowHeight());
	vulkanPipeline->SetActive(drawCmdBuffers[frameBufferId]);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(drawCmdBuffers[frameBufferId]->GetCommandBuffer(), 0, 1, &vertexBuffer, offsets);

	vkCmdDraw(drawCmdBuffers[frameBufferId]->GetCommandBuffer(), vertexCount, 1, 0, 0);
	drawCmdBuffers[frameBufferId]->EndRecording();
	drawCmdBuffers[frameBufferId]->ExecuteSecondary(commandBuffer);
}

void Canvas::SetPosition(float x, float y)
{
	if (posX != x || posY != y)
		updateVertexBuffer = true;

	posX = x;
	posY = y;
}

void Canvas::SetDimensions(float width, float height)
{
	if (this->width != width || this->height != height)
		updateVertexBuffer = true;

	this->width = width;
	this->height = height;
}

void Canvas::UpdateVertexData()
{
	// Bottom right
	vertexData[0].x = posX + width;
	vertexData[0].y = posY + height;
	vertexData[0].z = 0.0f;
	vertexData[0].u = 1.0f;
	vertexData[0].v = 1.0f;

	// Bottom left
	vertexData[1].x = posX;
	vertexData[1].y = posY + height;
	vertexData[1].z = 0.0f;
	vertexData[1].u = 0.0f;
	vertexData[1].v = 1.0f;

	// Top left
	vertexData[2].x = posX;
	vertexData[2].y = posY;
	vertexData[2].z = 0.0f;
	vertexData[2].u = 0.0f;
	vertexData[2].v = 0.0f;

	// Top right
	vertexData[3].x = posX + width;
	vertexData[3].y = posY;
	vertexData[3].z = 0.0f;
	vertexData[3].u = 1.0f;
	vertexData[3].v = 0.0f;

	// Bottom right
	vertexData[4].x = posX + width;
	vertexData[4].y = posY + height;
	vertexData[4].z = 0.0f;
	vertexData[4].u = 1.0f;
	vertexData[4].v = 1.0f;

	// Top left
	vertexData[5].x = posX;
	vertexData[5].y = posY;
	vertexData[5].z = 0.0f;
	vertexData[5].u = 0.0f;
	vertexData[5].v = 0.0f;
}

void Canvas::UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VkImageView * imageView)
{
	VkWriteDescriptorSet write[2];

	write[0] = {};
	write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[0].pNext = NULL;
	write[0].dstSet = vulkanPipeline->GetDescriptorSet();
	write[0].descriptorCount = 1;
	write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write[0].pBufferInfo = &vsUniformBufferInfo;
	write[0].dstArrayElement = 0;
	write[0].dstBinding = 0;

	VkDescriptorImageInfo positionTextureDesc{};
	positionTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	positionTextureDesc.imageView = *imageView;
	positionTextureDesc.sampler = vulkan->GetColorSampler();

	write[1] = {};
	write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[1].pNext = NULL;
	write[1].dstSet = vulkanPipeline->GetDescriptorSet();
	write[1].descriptorCount = 1;
	write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[1].pImageInfo = &positionTextureDesc;
	write[1].dstArrayElement = 0;
	write[1].dstBinding = 1;

	vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(write) / sizeof(write[0]), write, 0, NULL);
}