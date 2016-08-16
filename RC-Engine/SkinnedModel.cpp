/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkinnedModel.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>

#include "SkinnedModel.h"
#include "StdInc.h"
#include "LogManager.h"
#include "Timer.h"

extern LogManager * gLogManager;
extern Timer * gTimer;

SkinnedModel::SkinnedModel()
{
	vsUniformBuffer = VK_NULL_HANDLE;
	currentAnim = NULL;
}

SkinnedModel::~SkinnedModel()
{
	currentAnim = NULL;
	vsUniformBuffer = VK_NULL_HANDLE;
}

bool SkinnedModel::Init(std::string filename, VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VkResult result;

	VkMemoryAllocateInfo allocInfo{};
	uint8_t *pData;

	// Uniform buffer init
	vertexUniformBuffer.worldMatrix = glm::mat4(1.0f);
	vertexUniformBuffer.MVP = glm::mat4();
	for (unsigned int i = 0; i < MAX_BONES; i++)
		boneUniformBufferData.bones[i] = glm::mat4();

	// Vertex shader - Uniform buffer
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

	// Vertex shader - Bone Uniform buffer
	VkBufferCreateInfo boneBufferCI{};
	boneBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	boneBufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	boneBufferCI.size = sizeof(boneUniformBufferData);
	boneBufferCI.queueFamilyIndexCount = 0;
	boneBufferCI.pQueueFamilyIndices = VK_NULL_HANDLE;
	boneBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &boneBufferCI, VK_NULL_HANDLE, &boneUniformBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), boneUniformBuffer, &boneMemReq);

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = boneMemReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(vsMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &boneUniformMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), boneUniformMemory, 0, boneMemReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, &boneUniformBufferData, sizeof(boneUniformBufferData));

	vkUnmapMemory(vulkanDevice->GetDevice(), boneUniformMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), boneUniformBuffer, boneUniformMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	boneUniformBufferInfo.buffer = boneUniformBuffer;
	boneUniformBufferInfo.offset = 0;
	boneUniformBufferInfo.range = sizeof(boneUniformBufferData);

	// Open .rcs file
	FILE * file = fopen(filename.c_str(), "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Model file not found! (" + filename + ")");
		return false;
	}

	// Open .mat file
	size_t pos = filename.rfind('.');
	filename.replace(pos, 4, ".mat");

	std::ifstream matFile(filename.c_str());
	if (!matFile.is_open())
	{
		gLogManager->AddMessage("ERROR: Model .mat file not found! (" + filename + ")");
		return false;
	}

	unsigned int meshCount;
	fread(&meshCount, sizeof(unsigned int), 1, file);

	for (unsigned int i = 0; i < meshCount; i++)
	{
		// Create and read mesh data
		SkinnedMesh * mesh = new SkinnedMesh();
		if (!mesh->Init(vulkan, file))
		{
			gLogManager->AddMessage("ERROR: Failed to init a mesh!");
			return false;
		}
		meshes.push_back(mesh);

		std::string texturePath;
		char diffuseTextureName[64];
		char specularTextureName[64];

		// Read diffuse texture
		fread(diffuseTextureName, sizeof(char), 64, file);
		if (strcmp(diffuseTextureName, "NONE") == 0)
			texturePath = "data/textures/test.rct";
		else
			texturePath = "data/textures/" + std::string(diffuseTextureName);

		Texture * diffuse = new Texture();
		if (!diffuse->Init(vulkan->GetVulkanDevice(), cmdBuffer, texturePath))
		{
			gLogManager->AddMessage("ERROR: Couldn't init texture!");
			return false;
		}
		textures.push_back(diffuse);

		// Read specular texture
		fread(specularTextureName, sizeof(char), 64, file);
		if (strcmp(specularTextureName, "NONE") == 0)
			texturePath = "data/textures/spec.rct";
		else
			texturePath = "data/textures/" + std::string(specularTextureName);

		Texture * specular = new Texture();
		if (!specular->Init(vulkan->GetVulkanDevice(), cmdBuffer, texturePath))
		{
			gLogManager->AddMessage("ERROR: Couldn't init a texture!");
			return false;
		}
		textures.push_back(specular);

		// Init mesh material
		Material * material = new Material();
		material->SetDiffuseTexture(diffuse);
		material->SetSpecularTexture(specular);

		std::string matName;
		float specularStrength, specularShininess;
		matFile >> matName >> specularShininess >> specularStrength;
		
		material->SetSpecularShininess(specularShininess);
		material->SetSpecularStrength(specularStrength);

		materials.push_back(material);
		meshes[i]->SetMaterial(material);

		// Init draw command buffers for each meash
		VulkanCommandBuffer * drawCmdBuffer = new VulkanCommandBuffer();
		if (!drawCmdBuffer->Init(vulkanDevice, vulkan->GetVulkanCommandPool(), false))
		{
			gLogManager->AddMessage("ERROR: Failed to create a draw command buffer!");
			return false;
		}
		drawCmdBuffers.push_back(drawCmdBuffer);
	}

	matFile.close();

	// Read bone offsets
	fread(&numBones, sizeof(unsigned int), 1, file);
	boneOffsets.resize(numBones);
	fread(boneOffsets.data(), sizeof(aiMatrix4x4), numBones, file);

	// Read bone mappings
	for (unsigned int i = 0; i < numBones; i++)
	{
		unsigned int strSize;
		char * str;
		std::string boneName;
		uint32_t id;

		fread(&strSize, sizeof(unsigned int), 1, file);
		str = new char[strSize+1];
		fread(str, sizeof(char), strSize, file);
		fread(&id, sizeof(uint32_t), 1, file);

		str[strSize] = 0;
		boneName = str;
		boneMapping[boneName] = id;
	}

	fclose(file);

	return true;
}

void SkinnedModel::Unload(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();

	vkFreeMemory(vulkanDevice->GetDevice(), boneUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), boneUniformBuffer, VK_NULL_HANDLE);
	vkFreeMemory(vulkanDevice->GetDevice(), vsUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vsUniformBuffer, VK_NULL_HANDLE);

	for (unsigned int i = 0; i < textures.size(); i++)
		SAFE_UNLOAD(textures[i], vulkanDevice);

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		SAFE_UNLOAD(drawCmdBuffers[i], vulkanDevice, vulkan->GetVulkanCommandPool());
		SAFE_DELETE(materials[i]);
		SAFE_UNLOAD(meshes[i], vulkan);
	}
}

void SkinnedModel::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
	Camera * camera, ShadowMaps * shadowMaps)
{
	uint8_t *pData;

	vertexUniformBuffer.MVP = vulkan->GetProjectionMatrix() * camera->GetViewMatrix() * vertexUniformBuffer.worldMatrix;

	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory);

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		if (vulkanPipeline->GetPipelineName() == "SKINNED")
		{
			meshes[i]->UpdateUniformBuffer(vulkan);
			UpdateDescriptorSet(vulkan, vulkanPipeline, meshes[i]);

			// Record draw command
			drawCmdBuffers[i]->BeginRecordingSecondary(vulkan->GetDeferredRenderpass()->GetRenderpass(), vulkan->GetDeferredFramebuffer());

			vulkan->InitViewportAndScissors(drawCmdBuffers[i]);
			vulkanPipeline->SetActive(drawCmdBuffers[i]);
			meshes[i]->Render(vulkan, drawCmdBuffers[i]);

			drawCmdBuffers[i]->EndRecording();
			drawCmdBuffers[i]->ExecuteSecondary(commandBuffer);
		}
		else if (vulkanPipeline->GetPipelineName() == "SHADOWSKINNED")
		{
			UpdateDescriptorSet(vulkan, vulkanPipeline, meshes[i]);

			// Record draw command
			drawCmdBuffers[i]->BeginRecordingSecondary(shadowMaps->GetShadowRenderpass()->GetRenderpass(), shadowMaps->GetFramebuffer());

			vulkan->InitViewportAndScissors(drawCmdBuffers[i]);
			vulkanPipeline->SetActive(drawCmdBuffers[i]);
			meshes[i]->Render(vulkan, drawCmdBuffers[i]);

			drawCmdBuffers[i]->EndRecording();
			drawCmdBuffers[i]->ExecuteSecondary(commandBuffer);
		}
	}
}

void SkinnedModel::UpdateAnimation(VulkanInterface * vulkan)
{
	if (currentAnim != NULL)
	{
		uint8_t * pData;

		currentAnim->Update(gTimer->GetDelta(), boneOffsets, boneMapping);
	
		std::vector<glm::mat4> boneTransforms = currentAnim->GetBoneTransforms();
		memcpy(boneUniformBufferData.bones, boneTransforms.data(), sizeof(glm::mat4) * boneTransforms.size());

		vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), boneUniformMemory, 0, boneMemReq.size, 0, (void**)&pData);
		memcpy(pData, &boneUniformBufferData, sizeof(boneUniformBufferData));
		vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), boneUniformMemory);
	}
}

void SkinnedModel::SetWorldMatrix(glm::mat4 & worldMatrix)
{
	vertexUniformBuffer.worldMatrix = worldMatrix;
}

void SkinnedModel::SetAnimation(Animation * anim)
{
	currentAnim = anim;
}

void SkinnedModel::UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * pipeline, SkinnedMesh * mesh)
{
	if (pipeline->GetPipelineName() == "SKINNED")
	{
		VkWriteDescriptorSet descriptorWrite[5];

		descriptorWrite[0] = {};
		descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[0].pNext = NULL;
		descriptorWrite[0].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[0].descriptorCount = 1;
		descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[0].pBufferInfo = &vsUniformBufferInfo;
		descriptorWrite[0].dstArrayElement = 0;
		descriptorWrite[0].dstBinding = 0;

		descriptorWrite[1] = {};
		descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[1].pNext = NULL;
		descriptorWrite[1].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[1].descriptorCount = 1;
		descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[1].pBufferInfo = &boneUniformBufferInfo;
		descriptorWrite[1].dstArrayElement = 0;
		descriptorWrite[1].dstBinding = 1;

		// Write mesh diffuse texture
		VkDescriptorImageInfo diffuseTextureDesc{};
		diffuseTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		diffuseTextureDesc.imageView = *mesh->GetMaterial()->GetDiffuseTexture()->GetImageView();
		diffuseTextureDesc.sampler = vulkan->GetColorSampler();

		descriptorWrite[2] = {};
		descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[2].pNext = NULL;
		descriptorWrite[2].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[2].descriptorCount = 1;
		descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite[2].pImageInfo = &diffuseTextureDesc;
		descriptorWrite[2].dstArrayElement = 0;
		descriptorWrite[2].dstBinding = 2;

		// Write mesh specular texture
		VkDescriptorImageInfo specularTextureDesc{};
		specularTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		specularTextureDesc.imageView = *mesh->GetMaterial()->GetSpecularTexture()->GetImageView();
		specularTextureDesc.sampler = vulkan->GetColorSampler();

		descriptorWrite[3] = {};
		descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[3].pNext = NULL;
		descriptorWrite[3].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[3].descriptorCount = 1;
		descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite[3].pImageInfo = &specularTextureDesc;
		descriptorWrite[3].dstArrayElement = 0;
		descriptorWrite[3].dstBinding = 3;

		// Update material uniform buffer
		descriptorWrite[4] = {};
		descriptorWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[4].pNext = NULL;
		descriptorWrite[4].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[4].descriptorCount = 1;
		descriptorWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[4].pBufferInfo = mesh->GetMaterialBufferInfo();
		descriptorWrite[4].dstArrayElement = 0;
		descriptorWrite[4].dstBinding = 4;

		vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);
	}
	else if (pipeline->GetPipelineName() == "SHADOWSKINNED")
	{
		VkWriteDescriptorSet descriptorWrite[2];

		descriptorWrite[0] = {};
		descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[0].pNext = NULL;
		descriptorWrite[0].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[0].descriptorCount = 1;
		descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[0].pBufferInfo = &vsUniformBufferInfo;
		descriptorWrite[0].dstArrayElement = 0;
		descriptorWrite[0].dstBinding = 0;

		descriptorWrite[1] = {};
		descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[1].pNext = NULL;
		descriptorWrite[1].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[1].descriptorCount = 1;
		descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[1].pBufferInfo = &boneUniformBufferInfo;
		descriptorWrite[1].dstArrayElement = 0;
		descriptorWrite[1].dstBinding = 1;

		vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);
	}
}