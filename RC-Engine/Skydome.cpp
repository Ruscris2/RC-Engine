/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Skydome.cpp                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Skydome.h"
#include "StdInc.h"
#include "Settings.h"

extern Settings * gSettings;

Skydome::Skydome()
{
	vertexBuffer = NULL;
	indexBuffer = NULL;
	vsUBO = NULL;
	fsUBO = NULL;
}

Skydome::~Skydome()
{
	fsUBO = NULL;
	vsUBO = NULL;
	indexBuffer = NULL;
	vertexBuffer = NULL;
}

bool Skydome::Init(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	Vertex * vertexData;
	uint32_t * indexData;

	// Generate sphere
	float radius = 2.0f;
	unsigned int slices = 25;
	unsigned int stacks = 25;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Vertex vertex;
	vertex.x = 0.0f;
	vertex.y = radius;
	vertex.z = 0.0f;
	vertices.push_back(vertex);

	float phiStep = glm::pi<float>() / stacks;
	float thetaStep = 2.0f * glm::pi<float>() / slices;

	for (unsigned int i = 1; i <= stacks - 1; i++)
	{
		float phi = i * phiStep;
		for (unsigned int j = 0; j <= slices; j++)
		{
			float theta = j * thetaStep;
			vertex.x = radius * glm::sin(phi) * glm::cos(theta);
			vertex.y = radius * glm::cos(phi);
			vertex.z = radius * glm::sin(phi) * glm::sin(theta);
			vertices.push_back(vertex);
		}
	}

	vertex.x = 0.0f;
	vertex.y = -radius;
	vertex.z = 0.0f;
	vertices.push_back(vertex);

	for (unsigned int i = 1; i <= slices; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	int baseIndex = 1;
	int ringVertexCount = slices + 1;
	for (unsigned int i = 0; i < stacks - 2; i++)
	{
		for (unsigned int j = 0; j < slices; j++)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	int southPoleIndex = (int)vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (unsigned int i = 0; i < slices; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	vertexCount = (unsigned int)vertices.size();
	indexCount = (unsigned int)indices.size();

	vertexData = new Vertex[vertexCount];
	indexData = new uint32_t[indexCount];

	memcpy(vertexData, vertices.data(), sizeof(Vertex) * vertexCount);
	memcpy(indexData, indices.data(), sizeof(uint32_t) * indexCount);

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

	// Uniform buffer init
	vertexUniformBuffer.MVP = glm::mat4();
	fragmentUniformBuffer.skyColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	fragmentUniformBuffer.atmosphereColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	fragmentUniformBuffer.groundColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	fragmentUniformBuffer.atmosphereHeight = 0.0f;
	fragmentUniformBuffer.padding = glm::vec3(0.0f, 0.0f, 0.0f);

	// Vertex shader Uniform buffer
	vsUBO = new VulkanBuffer();
	if (!vsUBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &vertexUniformBuffer,
		sizeof(vertexUniformBuffer), false))
		return false;

	// Fragment shader uniform buffer
	fsUBO = new VulkanBuffer();
	if (!fsUBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &fragmentUniformBuffer,
		sizeof(fragmentUniformBuffer), false))
		return false;

	// Write descriptor set
	VkWriteDescriptorSet descriptorWrite[2];

	descriptorWrite[0] = {};
	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].pNext = NULL;
	descriptorWrite[0].dstSet = vulkanPipeline->GetDescriptorSet();
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[0].pBufferInfo = vsUBO->GetBufferInfo();
	descriptorWrite[0].dstArrayElement = 0;
	descriptorWrite[0].dstBinding = 0;

	descriptorWrite[1] = {};
	descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].pNext = NULL;
	descriptorWrite[1].dstSet = vulkanPipeline->GetDescriptorSet();
	descriptorWrite[1].descriptorCount = 1;
	descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[1].pBufferInfo = fsUBO->GetBufferInfo();
	descriptorWrite[1].dstArrayElement = 0;
	descriptorWrite[1].dstBinding = 1;

	vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);

	worldMatrix = glm::mat4(1.0f);

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

void Skydome::Unload(VulkanInterface * vulkan)
{
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
		SAFE_UNLOAD(drawCmdBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());

	SAFE_UNLOAD(fsUBO, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(vsUBO, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(indexBuffer, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(vertexBuffer, vulkan->GetVulkanDevice());
}

void Skydome::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * pipeline, Camera * camera, int framebufferId)
{
	// Update vertex uniform buffer
	glm::vec3 camPos = camera->GetPosition();
	camPos.y -= 0.25f;
	worldMatrix = glm::translate(glm::mat4(1.0f), camPos);
	vertexUniformBuffer.MVP = camera->GetProjectionMatrix() * camera->GetViewMatrix() * worldMatrix;

	vsUBO->Update(vulkan->GetVulkanDevice(), &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	// Update fragment uniform buffer
	fragmentUniformBuffer.skyColor = skyColor;
	fragmentUniformBuffer.atmosphereColor = atmosphereColor;
	fragmentUniformBuffer.groundColor = groundColor;
	fragmentUniformBuffer.atmosphereHeight = atmosphereHeight;

	fsUBO->Update(vulkan->GetVulkanDevice(), &fragmentUniformBuffer, sizeof(fragmentUniformBuffer));

	// Render
	drawCmdBuffers[framebufferId]->BeginRecordingSecondary(vulkan->GetForwardRenderpass()->GetRenderpass(), vulkan->GetVulkanSwapchain()->GetFramebuffer(framebufferId));
	vulkan->InitViewportAndScissors(drawCmdBuffers[framebufferId], (float)gSettings->GetWindowWidth(), (float)gSettings->GetWindowHeight(),
		(uint32_t)gSettings->GetWindowWidth(), (uint32_t)gSettings->GetWindowHeight());
	pipeline->SetActive(drawCmdBuffers[framebufferId]);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(drawCmdBuffers[framebufferId]->GetCommandBuffer(), 0, 1, vertexBuffer->GetBuffer(), offsets);
	vkCmdBindIndexBuffer(drawCmdBuffers[framebufferId]->GetCommandBuffer(), *indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(drawCmdBuffers[framebufferId]->GetCommandBuffer(), indexCount, 1, 0, 0, 0);

	drawCmdBuffers[framebufferId]->EndRecording();
	drawCmdBuffers[framebufferId]->ExecuteSecondary(commandBuffer);
}

void Skydome::SetSkyColor(float r, float g, float b, float a)
{
	skyColor = glm::vec4(r, g, b, a);
}

void Skydome::SetAtmosphereColor(float r, float g, float b, float a)
{
	atmosphereColor = glm::vec4(r, g, b, a);
}

void Skydome::SetGroundColor(float r, float g, float b, float a)
{
	groundColor = glm::vec4(r, g, b, a);
}

void Skydome::SetAtmosphereHeight(float height)
{
	atmosphereHeight = height;
}

