/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkinnedMesh.cpp                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "SkinnedMesh.h"
#include "StdInc.h"
#include "BufferManager.h"

extern BufferManager * gBufferManager;

SkinnedMesh::SkinnedMesh()
{
	vertexBuffer = NULL;
	indexBuffer = NULL;
}

SkinnedMesh::~SkinnedMesh()
{
	indexBuffer = NULL;
	vertexBuffer = NULL;
}

bool SkinnedMesh::Init(VulkanInterface * vulkan, FILE * modelFile, std::string meshName)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	fread(&vertexCount, sizeof(unsigned int), 1, modelFile);
	fread(&indexCount, sizeof(unsigned int), 1, modelFile);

	Vertex * vertexData = new Vertex[vertexCount];
	uint32_t * indexData = new uint32_t[indexCount];

	fread(vertexData, sizeof(Vertex), vertexCount, modelFile);
	fread(indexData, sizeof(uint32_t), indexCount, modelFile);

	// Command buffer used for creating buffers
	VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
	if (!cmdBuffer->Init(vulkanDevice, cmdPool, true))
		return false;

	cmdBuffer->BeginRecording();

	// Vertex buffer
	vertexBuffer = gBufferManager->RequestBuffer(meshName + "VB", vulkanDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		vertexData, sizeof(Vertex) * vertexCount, true, cmdBuffer);
	if (vertexBuffer == nullptr)
		return false;

	// Index buffer
	indexBuffer = gBufferManager->RequestBuffer(meshName + "IB", vulkanDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		indexData, sizeof(uint32_t) * indexCount, true, cmdBuffer);
	if (indexBuffer == nullptr)
		return false;

	cmdBuffer->EndRecording();
	cmdBuffer->Execute(vulkanDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, true);

	SAFE_UNLOAD(cmdBuffer, vulkanDevice, cmdPool);

	delete[] vertexData;
	delete[] indexData;

	// Material uniform buffer
	materialUniformBuffer.hasNormalMap = 0.0f;
	materialUniformBuffer.metallicOffset = 0.0f;
	materialUniformBuffer.roughnessOffset = 0.0f;
	materialUniformBuffer.padding = 0.0f;

	// Fragment shader uniform buffer
	materialUBO = new VulkanBuffer();
	if (!materialUBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &materialUniformBuffer,
		sizeof(materialUniformBuffer), false))
		return false;

	return true;
}

void SkinnedMesh::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(materialUBO, vulkan->GetVulkanDevice());
	gBufferManager->ReleaseBuffer(indexBuffer, vulkan->GetVulkanDevice());
	gBufferManager->ReleaseBuffer(vertexBuffer, vulkan->GetVulkanDevice());
}

void SkinnedMesh::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer)
{
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer->GetCommandBuffer(), 0, 1, vertexBuffer->GetBuffer(), offsets);
	vkCmdBindIndexBuffer(commandBuffer->GetCommandBuffer(), *indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer->GetCommandBuffer(), indexCount, 1, 0, 0, 0);
}

void SkinnedMesh::UpdateUniformBuffer(VulkanInterface * vulkan)
{
	materialUniformBuffer.hasNormalMap = (material->HasNormalMap() ? 1.0f : 0.0f);
	materialUniformBuffer.metallicOffset = material->GetMetallicOffset();
	materialUniformBuffer.roughnessOffset = material->GetRoughnessOffset();

	materialUBO->Update(vulkan->GetVulkanDevice(), &materialUniformBuffer, sizeof(materialUniformBuffer));
}

void SkinnedMesh::SetMaterial(Material * material)
{
	this->material = material;
}

Material * SkinnedMesh::GetMaterial()
{
	return material;
}

VkDescriptorBufferInfo * SkinnedMesh::GetMaterialBufferInfo()
{
	return materialUBO->GetBufferInfo();
}
