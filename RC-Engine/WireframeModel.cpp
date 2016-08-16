/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: WireframeModel.cpp                                   |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "WireframeModel.h"
#include "StdInc.h"

WireframeModel::WireframeModel()
{
	vertexBuffer = VK_NULL_HANDLE;
	indexBuffer = VK_NULL_HANDLE;

	posX = posY = posZ = 0.0f;
	rotX = rotY = rotZ = 0.0f;
}

WireframeModel::~WireframeModel()
{
	indexBuffer = VK_NULL_HANDLE;
	vertexBuffer = VK_NULL_HANDLE;
}

bool WireframeModel::Init(VulkanInterface * vulkan, GEOMETRY_GENERATE_INFO generateInfo, glm::vec4 color)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VulkanCommandPool * cmdPool = vulkan->GetVulkanCommandPool();

	VkResult result;
	VkMemoryRequirements memReq;
	VkMemoryAllocateInfo allocInfo{};
	uint8_t *pData;

	Vertex * vertexData;
	uint32_t * indexData;

	if (generateInfo.type == GEOMETRY_TYPE_BOX)
	{
		if (generateInfo.width == 0.0f || generateInfo.height == 0.0f || generateInfo.length == 0.0f)
			return false;

		vertexCount = 8;
		indexCount = 36;

		vertexData = new Vertex[vertexCount];
		indexData = new uint32_t[indexCount];

		float halfWidth = generateInfo.width / 2;
		float halfHeight = generateInfo.height / 2;
		float halfLength = generateInfo.length / 2;

		vertexData[0].x = -halfWidth;
		vertexData[0].y = halfHeight;
		vertexData[0].z = -halfLength;

		vertexData[1].x = halfWidth;
		vertexData[1].y = halfHeight;
		vertexData[1].z = -halfLength;

		vertexData[2].x = halfWidth;
		vertexData[2].y = -halfHeight;
		vertexData[2].z = -halfLength;

		vertexData[3].x = -halfWidth;
		vertexData[3].y = -halfHeight;
		vertexData[3].z = -halfLength;

		vertexData[4].x = -halfWidth;
		vertexData[4].y = halfHeight;
		vertexData[4].z = halfLength;

		vertexData[5].x = halfWidth;
		vertexData[5].y = halfHeight;
		vertexData[5].z = halfLength;

		vertexData[6].x = halfWidth;
		vertexData[6].y = -halfHeight;
		vertexData[6].z = halfLength;

		vertexData[7].x = -halfWidth;
		vertexData[7].y = -halfHeight;
		vertexData[7].z = halfLength;

		indexData[0] = 0; indexData[1] = 2; indexData[2] = 3;
		indexData[3] = 0; indexData[4] = 1; indexData[5] = 2;
		indexData[6] = 4; indexData[7] = 6; indexData[8] = 7;
		indexData[9] = 4; indexData[10] = 5; indexData[11] = 6;
		indexData[12] = 4; indexData[13] = 1; indexData[14] = 0;
		indexData[15] = 4; indexData[16] = 5; indexData[17] = 1;
		indexData[18] = 7; indexData[19] = 2; indexData[20] = 3;
		indexData[21] = 7; indexData[22] = 6; indexData[23] = 2;
		indexData[24] = 4; indexData[25] = 3; indexData[26] = 7;
		indexData[27] = 4; indexData[28] = 0; indexData[29] = 3;
		indexData[30] = 5; indexData[31] = 2; indexData[32] = 6;
		indexData[33] = 5; indexData[34] = 1; indexData[35] = 2;
	}
	else if (generateInfo.type == GEOMETRY_TYPE_SPHERE)
	{
		if (generateInfo.radius == 0.0f || generateInfo.slices == 0 || generateInfo.stacks == 0)
			return false;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Vertex vertex;
		vertex.x = 0.0f;
		vertex.y = generateInfo.radius;
		vertex.z = 0.0f;
		vertices.push_back(vertex);

		float phiStep = glm::pi<float>() / generateInfo.stacks;
		float thetaStep = 2.0f * glm::pi<float>() / generateInfo.slices;

		for (unsigned int i = 1; i <= generateInfo.stacks - 1; i++)
		{
			float phi = i * phiStep;
			for (unsigned int j = 0; j <= generateInfo.slices; j++)
			{
				float theta = j * thetaStep;
				vertex.x = generateInfo.radius * glm::sin(phi) * glm::cos(theta);
				vertex.y = generateInfo.radius * glm::cos(phi);
				vertex.z = generateInfo.radius * glm::sin(phi) * glm::sin(theta);
				vertices.push_back(vertex);
			}
		}

		vertex.x = 0.0f;
		vertex.y = -generateInfo.radius;
		vertex.z = 0.0f;
		vertices.push_back(vertex);

		for (unsigned int i = 1; i <= generateInfo.slices; i++)
		{
			indices.push_back(0);
			indices.push_back(i + 1);
			indices.push_back(i);
		}

		int baseIndex = 1;
		int ringVertexCount = generateInfo.slices + 1;
		for (unsigned int i = 0; i < generateInfo.stacks - 2; i++)
		{
			for (unsigned int j = 0; j < generateInfo.slices; j++)
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
		for (unsigned int i = 0; i < generateInfo.slices; i++)
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
	}
	else if (generateInfo.type == GEOMETRY_TYPE_POLYGON)
	{
		vertexCount = 4;
		indexCount = 6;

		vertexData = new Vertex[vertexCount];
		indexData = new uint32_t[indexCount];
		
		vertexData[0].x = generateInfo.v1.x;
		vertexData[0].y = generateInfo.v1.y;
		vertexData[0].z = generateInfo.v1.z;

		vertexData[1].x = generateInfo.v2.x;
		vertexData[1].y = generateInfo.v2.y;
		vertexData[1].z = generateInfo.v2.z;

		vertexData[2].x = generateInfo.v3.x;
		vertexData[2].y = generateInfo.v3.y;
		vertexData[2].z = generateInfo.v3.z;

		vertexData[3].x = generateInfo.v4.x;
		vertexData[3].y = generateInfo.v4.y;
		vertexData[3].z = generateInfo.v4.z;

		indexData[0] = 0; indexData[1] = 2; indexData[2] = 3;
		indexData[3] = 0; indexData[4] = 1; indexData[5] = 2;
	}
	else if (generateInfo.type == GEOMETRY_TYPE_CYLINDER)
	{
		if (generateInfo.radius == 0.0f || generateInfo.height == 0.0f || generateInfo.stacks == 0 || generateInfo.slices == 0)
			return false;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		// Body
		float stackHeight = generateInfo.height / generateInfo.stacks;
		unsigned int ringCount = generateInfo.stacks + 1;

		for (unsigned int i = 0; i < ringCount; i++)
		{
			float y = -0.5f * generateInfo.height + i * stackHeight;
			float theta = 2.0f * glm::pi<float>() / generateInfo.slices;

			for (unsigned int j = 0; j <= generateInfo.slices; j++)
			{
				Vertex vertex;

				vertex.x = glm::cos(j * theta) * generateInfo.radius;
				vertex.y = y;
				vertex.z = glm::sin(j * theta) * generateInfo.radius;
				vertices.push_back(vertex);
			}
		}

		unsigned int ringVertexCount = generateInfo.slices + 1;
		for (unsigned int i = 0; i < generateInfo.stacks; i++)
		{
			for (unsigned int j = 0; j < generateInfo.slices; j++)
			{
				indices.push_back(i * ringVertexCount + j);
				indices.push_back((i + 1) * ringVertexCount + j);
				indices.push_back((i + 1) * ringVertexCount + j + 1);

				indices.push_back(i * ringVertexCount + j);
				indices.push_back((i + 1) * ringVertexCount + j + 1);
				indices.push_back(i * ringVertexCount + j + 1);
			}
		}

		Vertex vertex;
		float y;
		unsigned int baseIndex, centerIndex;

		float theta = 2.0f * glm::pi<float>() / generateInfo.slices;;

		// Top cap
		baseIndex = (unsigned int)vertices.size();

		y = 0.5f * generateInfo.height;

		for (unsigned int i = 0; i <= generateInfo.slices; i++)
		{
			vertex.x = generateInfo.radius * glm::cos(i * theta);
			vertex.y = y;
			vertex.z = generateInfo.radius * glm::sin(i * theta);
			vertices.push_back(vertex);
		}

		vertex.x = 0.0f;
		vertex.y = y;
		vertex.z = 0.0f;
		vertices.push_back(vertex);

		centerIndex = (unsigned int)vertices.size() - 1;
		for (unsigned int i = 0; i < generateInfo.slices; i++)
		{
			indices.push_back(centerIndex);
			indices.push_back(baseIndex + i + 1);
			indices.push_back(baseIndex + i);
		}

		// Bottom cap
		baseIndex = (unsigned int)vertices.size();

		y = -0.5f * generateInfo.height;
		
		for (unsigned int i = 0; i <= generateInfo.slices; i++)
		{
			vertex.x = generateInfo.radius * glm::cos(i * theta);
			vertex.y = y;
			vertex.z = generateInfo.radius * glm::sin(i * theta);
			vertices.push_back(vertex);
		}

		vertex.x = 0.0f;
		vertex.y = y;
		vertex.z = 0.0f;
		vertices.push_back(vertex);

		centerIndex = (unsigned int)vertices.size() - 1;
		for (unsigned int i = 0; i < generateInfo.slices; i++)
		{
			indices.push_back(centerIndex);
			indices.push_back(baseIndex + i);
			indices.push_back(baseIndex + i + 1);
		}

		vertexCount = (unsigned int)vertices.size();
		indexCount = (unsigned int)indices.size();

		vertexData = new Vertex[vertexCount];
		indexData = new uint32_t[indexCount];

		memcpy(vertexData, vertices.data(), sizeof(Vertex) * vertexCount);
		memcpy(indexData, indices.data(), sizeof(uint32_t) * indexCount);
	}
	else
		return false;

	for (unsigned int i = 0; i < vertexCount; i++)
	{
		vertexData[i].r = color.r;
		vertexData[i].g = color.g;
		vertexData[i].b = color.b;
		vertexData[i].a = color.a;
	}

	// Vertex buffer
	VkBuffer stagingVertexBuffer;
	VkDeviceMemory stagingVertexMemory;

	VkBufferCreateInfo vertexBufferCI{};
	vertexBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	vertexBufferCI.size = sizeof(Vertex) * vertexCount;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vertexBufferCI, VK_NULL_HANDLE, &stagingVertexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), stagingVertexBuffer, &memReq);

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &stagingVertexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), stagingVertexMemory, 0, memReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, vertexData, sizeof(Vertex) * vertexCount);

	vkUnmapMemory(vulkanDevice->GetDevice(), stagingVertexMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), stagingVertexBuffer, stagingVertexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	vertexBufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vertexBufferCI, VK_NULL_HANDLE, &vertexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), vertexBuffer, &memReq);

	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &vertexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), vertexBuffer, vertexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	// Index buffer
	VkBuffer stagingIndexBuffer;
	VkDeviceMemory stagingIndexMemory;

	VkBufferCreateInfo indexBufferCI{};
	indexBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	indexBufferCI.size = sizeof(uint32_t) * indexCount;

	result = vkCreateBuffer(vulkanDevice->GetDevice(), &indexBufferCI, VK_NULL_HANDLE, &stagingIndexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), stagingIndexBuffer, &memReq);

	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &stagingIndexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), stagingIndexMemory, 0, memReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, indexData, sizeof(uint32_t) * indexCount);

	vkUnmapMemory(vulkanDevice->GetDevice(), stagingIndexMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), stagingIndexBuffer, stagingIndexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	indexBufferCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &indexBufferCI, VK_NULL_HANDLE, &indexBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), indexBuffer, &memReq);

	allocInfo.allocationSize = memReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &indexMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), indexBuffer, indexMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	// Copy data to VRAM using command buffer
	VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
	if (!cmdBuffer->Init(vulkanDevice, cmdPool, true))
		return false;

	cmdBuffer->BeginRecording();

	VkBufferCopy copyRegion{};
	copyRegion.size = sizeof(Vertex) * vertexCount;
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingVertexBuffer, vertexBuffer, 1, &copyRegion);
	copyRegion.size = sizeof(uint32_t) * indexCount;
	vkCmdCopyBuffer(cmdBuffer->GetCommandBuffer(), stagingIndexBuffer, indexBuffer, 1, &copyRegion);

	cmdBuffer->EndRecording();
	cmdBuffer->Execute(vulkanDevice, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, true);

	SAFE_UNLOAD(cmdBuffer, vulkanDevice, cmdPool);

	vkFreeMemory(vulkanDevice->GetDevice(), stagingVertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingVertexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), stagingIndexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), stagingIndexBuffer, VK_NULL_HANDLE);

	delete[] vertexData;
	delete[] indexData;

	// Uniform buffer init
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

void WireframeModel::Unload(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();

	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
		SAFE_UNLOAD(drawCmdBuffers[i], vulkanDevice, vulkan->GetVulkanCommandPool());

	vkFreeMemory(vulkanDevice->GetDevice(), vsUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vsUniformBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), indexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), indexBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), vertexMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vertexBuffer, VK_NULL_HANDLE);
}

void WireframeModel::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * pipeline, Camera * camera, int framebufferId)
{
	uint8_t *pData;

	// Update vertex uniform buffer
	vertexUniformBuffer.MVP = vulkan->GetProjectionMatrix() * camera->GetViewMatrix() * worldMatrix;

	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory);

	UpdateDescriptorSet(vulkan, pipeline);

	// Render
	drawCmdBuffers[framebufferId]->BeginRecordingSecondary(vulkan->GetForwardRenderpass()->GetRenderpass(), vulkan->GetVulkanSwapchain()->GetFramebuffer(framebufferId));
	vulkan->InitViewportAndScissors(drawCmdBuffers[framebufferId]);
	pipeline->SetActive(drawCmdBuffers[framebufferId]);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(drawCmdBuffers[framebufferId]->GetCommandBuffer(), 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(drawCmdBuffers[framebufferId]->GetCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(drawCmdBuffers[framebufferId]->GetCommandBuffer(), indexCount, 1, 0, 0, 0);

	drawCmdBuffers[framebufferId]->EndRecording();
	drawCmdBuffers[framebufferId]->ExecuteSecondary(commandBuffer);
}

void WireframeModel::SetPosition(float x, float y, float z)
{
	posX = x;
	posY = y;
	posZ = z;
	UpdateWorldMatrix();
}

void WireframeModel::SetRotation(float x, float y, float z)
{
	rotX = x;
	rotY = y;
	rotZ = z;
	UpdateWorldMatrix();
}

void WireframeModel::UpdateWorldMatrix()
{
	worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, posZ));
	worldMatrix = glm::rotate(worldMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
}

void WireframeModel::UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * pipeline)
{
	VkWriteDescriptorSet descriptorWrite[1];

	descriptorWrite[0] = {};
	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].pNext = NULL;
	descriptorWrite[0].dstSet = pipeline->GetDescriptorSet();
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[0].pBufferInfo = &vsUniformBufferInfo;
	descriptorWrite[0].dstArrayElement = 0;
	descriptorWrite[0].dstBinding = 0;

	vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);
}