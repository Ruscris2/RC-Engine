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
	vertexBuffer = NULL;
	vsUBO = NULL;

	posX = posY = 0.0f;
	width = height = 0.25f;
}

Canvas::~Canvas()
{
	delete[] vertexData;

	vsUBO = NULL;
	vertexBuffer = NULL;
}

bool Canvas::Init(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	vertexCount = 6;
	vertexData = new Vertex[vertexCount];
	UpdateVertexData();

	// Vertex buffer
	vertexBuffer = new VulkanBuffer();
	if (!vertexBuffer->Init(vulkanDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexData, sizeof(Vertex) * vertexCount, false))
		return false;

	// Uniform inits
	vertexUniformBuffer.MVP = glm::mat4();

	// Vertex shader Uniform buffer
	vsUBO = new VulkanBuffer();
	if (!vsUBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &vertexUniformBuffer,
		sizeof(vertexUniformBuffer), false))
		return false;

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
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
		SAFE_UNLOAD(drawCmdBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());

	SAFE_UNLOAD(vsUBO, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(vertexBuffer, vulkan->GetVulkanDevice());
}

void Canvas::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
	glm::mat4 orthoMatrix, VkImageView * imageView, int frameBufferId)
{
	// Update vertex buffer if needed
	if (updateVertexBuffer)
	{
		UpdateVertexData();
		vertexBuffer->Update(vulkan->GetVulkanDevice(), vertexData, sizeof(Vertex) * vertexCount);
		updateVertexBuffer = false;
	}

	UpdateDescriptorSet(vulkan, vulkanPipeline, imageView);

	// Update vertex uniform buffer
	vertexUniformBuffer.MVP = orthoMatrix;

	vsUBO->Update(vulkan->GetVulkanDevice(), &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	// Draw
	drawCmdBuffers[frameBufferId]->BeginRecordingSecondary(vulkan->GetForwardRenderpass()->GetRenderpass(), vulkan->GetVulkanSwapchain()->GetFramebuffer((int)frameBufferId));
	vulkan->InitViewportAndScissors(drawCmdBuffers[frameBufferId], (float)gSettings->GetWindowWidth(), (float)gSettings->GetWindowHeight(),
		(uint32_t)gSettings->GetWindowWidth(), (uint32_t)gSettings->GetWindowHeight());
	vulkanPipeline->SetActive(drawCmdBuffers[frameBufferId]);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(drawCmdBuffers[frameBufferId]->GetCommandBuffer(), 0, 1, vertexBuffer->GetBuffer(), offsets);

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
	write[0].pBufferInfo = vsUBO->GetBufferInfo();
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