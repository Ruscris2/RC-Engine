/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: RenderDummy.cpp                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "RenderDummy.h"
#include "StdInc.h"
#include "Settings.h"

extern Settings * gSettings;

RenderDummy::RenderDummy()
{
	vertexBuffer = NULL;
	indexBuffer = NULL;
	vsUBO = NULL;
	fsUBO = NULL;
}

RenderDummy::~RenderDummy()
{
	fsUBO = NULL;
	vsUBO = NULL;
	indexBuffer = NULL;
	vertexBuffer = NULL;
}

bool RenderDummy::Init(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VkImageView * positionView, VkImageView * normalView,
	VkImageView * albedoView, VkImageView * materialView, VkImageView * depthView, ShadowMaps * shadowMaps)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	vertexCount = 4;
	indexCount = 6;

	Vertex * vertexData = new Vertex[vertexCount];
	uint32_t * indexData = new uint32_t[indexCount];

	// Bottom right
	vertexData[0].x = 1.0f;
	vertexData[0].y = 1.0f;
	vertexData[0].z = -1.0f;
	vertexData[0].u = 1.0f;
	vertexData[0].v = 1.0f;

	// Bottom left
	vertexData[1].x = 0.0f;
	vertexData[1].y = 1.0f;
	vertexData[1].z = -1.0f;
	vertexData[1].u = 0.0f;
	vertexData[1].v = 1.0f;

	// Top left
	vertexData[2].x = 0.0f;
	vertexData[2].y = 0.0f;
	vertexData[2].z = -1.0f;
	vertexData[2].u = 0.0f;
	vertexData[2].v = 0.0f;

	// Top right
	vertexData[3].x = 1.0f;
	vertexData[3].y = 0.0f;
	vertexData[3].z = -1.0f;
	vertexData[3].u = 1.0f;
	vertexData[3].v = 0.0f;

	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;

	indexData[3] = 2;
	indexData[4] = 3;
	indexData[5] = 0;

	// Command buffer used for creating buffers
	VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
	if (!cmdBuffer->Init(vulkanDevice, cmdPool, true))
		return false;

	cmdBuffer->BeginRecording();

	// Vertex buffer
	vertexBuffer = new VulkanBuffer();
	if (!vertexBuffer->Init(vulkanDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexData,
		sizeof(Vertex) * vertexCount, true, cmdBuffer))
		return false;

	// Index buffer
	indexBuffer = new VulkanBuffer();
	if (!indexBuffer->Init(vulkanDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexData,
		sizeof(uint32_t) * indexCount, true, cmdBuffer))
		return false;

	cmdBuffer->EndRecording();
	cmdBuffer->Execute(vulkanDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, true);

	SAFE_UNLOAD(cmdBuffer, vulkanDevice, cmdPool);

	delete[] vertexData;
	delete[] indexData;

	// Uniform inits
	vertexUniformBuffer.MVP = glm::mat4();
	fragmentUniformBuffer.ambientColor = glm::vec4();
	fragmentUniformBuffer.diffuseColor = glm::vec4();
	fragmentUniformBuffer.specularColor = glm::vec4();
	fragmentUniformBuffer.lightDirection = glm::vec3();
	fragmentUniformBuffer.imageIndex = 5;
	fragmentUniformBuffer.cameraPosition = glm::vec3();
	fragmentUniformBuffer.lightViewMatrix = glm::mat4();
	fragmentUniformBuffer.padding = 0.0f;

	// Vertex shader Uniform buffer
	vsUBO = new VulkanBuffer();
	if (!vsUBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &vertexUniformBuffer,
		sizeof(vertexUniformBuffer), false))
		return false;

	// Fragment shader Uniform buffer
	fsUBO = new VulkanBuffer();
	if (!fsUBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &fragmentUniformBuffer,
		sizeof(fragmentUniformBuffer), false))
		return false;

	VkWriteDescriptorSet write[8];

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
	positionTextureDesc.imageView = *positionView;
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

	VkDescriptorImageInfo normalTextureDesc{};
	normalTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	normalTextureDesc.imageView = *normalView;
	normalTextureDesc.sampler = vulkan->GetColorSampler();

	write[2] = {};
	write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[2].pNext = NULL;
	write[2].dstSet = vulkanPipeline->GetDescriptorSet();
	write[2].descriptorCount = 1;
	write[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[2].pImageInfo = &normalTextureDesc;
	write[2].dstArrayElement = 0;
	write[2].dstBinding = 2;

	VkDescriptorImageInfo albedoTextureDesc{};
	albedoTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	albedoTextureDesc.imageView = *albedoView;
	albedoTextureDesc.sampler = vulkan->GetColorSampler();

	write[3] = {};
	write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[3].pNext = NULL;
	write[3].dstSet = vulkanPipeline->GetDescriptorSet();
	write[3].descriptorCount = 1;
	write[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[3].pImageInfo = &albedoTextureDesc;
	write[3].dstArrayElement = 0;
	write[3].dstBinding = 3;

	VkDescriptorImageInfo materialTextureDesc{};
	materialTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	materialTextureDesc.imageView = *materialView;
	materialTextureDesc.sampler = vulkan->GetColorSampler();

	write[4] = {};
	write[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[4].pNext = NULL;
	write[4].dstSet = vulkanPipeline->GetDescriptorSet();
	write[4].descriptorCount = 1;
	write[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[4].pImageInfo = &materialTextureDesc;
	write[4].dstArrayElement = 0;
	write[4].dstBinding = 4;

	VkDescriptorImageInfo depthTextureDesc{};
	depthTextureDesc.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthTextureDesc.imageView = *depthView;
	depthTextureDesc.sampler = vulkan->GetColorSampler();

	write[5] = {};
	write[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[5].pNext = NULL;
	write[5].dstSet = vulkanPipeline->GetDescriptorSet();
	write[5].descriptorCount = 1;
	write[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[5].pImageInfo = &depthTextureDesc;
	write[5].dstArrayElement = 0;
	write[5].dstBinding = 5;

	write[6] = {};
	write[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[6].pNext = NULL;
	write[6].dstSet = vulkanPipeline->GetDescriptorSet();
	write[6].descriptorCount = 1;
	write[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write[6].pBufferInfo = fsUBO->GetBufferInfo();
	write[6].dstArrayElement = 0;
	write[6].dstBinding = 6;

	VkDescriptorImageInfo shadowTextureDesc{};
	shadowTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	shadowTextureDesc.imageView = *shadowMaps->GetImageView();
	shadowTextureDesc.sampler = shadowMaps->GetSampler();

	write[7] = {};
	write[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write[7].pNext = NULL;
	write[7].dstSet = vulkanPipeline->GetDescriptorSet();
	write[7].descriptorCount = 1;
	write[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write[7].pImageInfo = &shadowTextureDesc;
	write[7].dstArrayElement = 0;
	write[7].dstBinding = 7;

	vkUpdateDescriptorSets(vulkanDevice->GetDevice(), sizeof(write) / sizeof(write[0]), write, 0, NULL);

	// Init draw command buffers
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
		if (!cmdBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), false))
			return false;

		drawCmdBuffers.push_back(cmdBuffer);
	}

	return true;
}

void RenderDummy::Unload(VulkanInterface * vulkan)
{
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
		SAFE_UNLOAD(drawCmdBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());

	SAFE_UNLOAD(fsUBO, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(vsUBO, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(indexBuffer, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(vertexBuffer, vulkan->GetVulkanDevice());
}

void RenderDummy::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
	glm::mat4 orthoMatrix, Light * light, int imageIndex, Camera * camera, ShadowMaps * shadowMaps, int frameBufferId)
{
	// Update vertex uniform buffer
	vertexUniformBuffer.MVP = orthoMatrix;
	
	vsUBO->Update(vulkan->GetVulkanDevice(), &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	// Update fragment uniform buffer
	fragmentUniformBuffer.ambientColor = light->GetAmbientColor();
	fragmentUniformBuffer.diffuseColor = light->GetDiffuseColor();
	fragmentUniformBuffer.specularColor = light->GetSpecularColor();
	fragmentUniformBuffer.lightDirection = light->GetLightDirection();
	fragmentUniformBuffer.imageIndex = imageIndex;
	fragmentUniformBuffer.cameraPosition = camera->GetPosition();

	fragmentUniformBuffer.lightViewMatrix = (shadowMaps->GetOrthoMatrix() * shadowMaps->GetViewMatrix());
	fragmentUniformBuffer.padding = 0.0f;

	fsUBO->Update(vulkan->GetVulkanDevice(), &fragmentUniformBuffer, sizeof(fragmentUniformBuffer));

	// Draw
	drawCmdBuffers[frameBufferId]->BeginRecordingSecondary(vulkan->GetForwardRenderpass()->GetRenderpass(), vulkan->GetVulkanSwapchain()->GetFramebuffer((int)frameBufferId));
	vulkan->InitViewportAndScissors(drawCmdBuffers[frameBufferId], (float)gSettings->GetWindowWidth(), (float)gSettings->GetWindowHeight(),
		(uint32_t)gSettings->GetWindowWidth(), (uint32_t)gSettings->GetWindowHeight());
	vulkanPipeline->SetActive(drawCmdBuffers[frameBufferId]);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(drawCmdBuffers[frameBufferId]->GetCommandBuffer(), 0, 1, vertexBuffer->GetBuffer(), offsets);
	vkCmdBindIndexBuffer(drawCmdBuffers[frameBufferId]->GetCommandBuffer(), *indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(drawCmdBuffers[frameBufferId]->GetCommandBuffer(), indexCount, 1, 0, 0, 0);

	drawCmdBuffers[frameBufferId]->EndRecording();
	drawCmdBuffers[frameBufferId]->ExecuteSecondary(commandBuffer);
}
