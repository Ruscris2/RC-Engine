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
#include "Settings.h"
#include "TextureManager.h"

extern LogManager * gLogManager;
extern Timer * gTimer;
extern Settings * gSettings;
extern TextureManager * gTextureManager;

SkinnedModel::SkinnedModel()
{
	skinnedVS_UBO = NULL;
	skinnedVS_bone_UBO = NULL;
	currentAnim = NULL;
}

SkinnedModel::~SkinnedModel()
{
	currentAnim = NULL;
	skinnedVS_bone_UBO = NULL;
	skinnedVS_UBO = NULL;
}

bool SkinnedModel::Init(std::string filename, VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();

	// Uniform buffer init
	vertexUniformBuffer.worldMatrix = glm::mat4(1.0f);
	vertexUniformBuffer.MVP = glm::mat4();
	for (unsigned int i = 0; i < MAX_BONES; i++)
		boneUniformBufferData.bones[i] = glm::mat4();

	// Vertex shader - Uniform buffer
	skinnedVS_UBO = new VulkanBuffer();
	if (!skinnedVS_UBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &vertexUniformBuffer,
		sizeof(vertexUniformBuffer), false))
		return false;

	// Vertex shader - Bone Uniform buffer
	skinnedVS_bone_UBO = new VulkanBuffer();
	if (!skinnedVS_bone_UBO->Init(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &boneUniformBufferData,
		sizeof(boneUniformBufferData), false))
		return false;

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
		char normalTextureName[64];

		// Init mesh material
		Material * material = new Material();

		// Read diffuse texture
		fread(diffuseTextureName, sizeof(char), 64, file);
		if (strcmp(diffuseTextureName, "NONE") == 0)
			texturePath = "data/textures/default_diffuse.rct";
		else
			texturePath = "data/textures/" + std::string(diffuseTextureName);

		Texture * diffuse = gTextureManager->RequestTexture(texturePath, vulkan->GetVulkanDevice(), cmdBuffer);
		if (diffuse == nullptr)
			return false;

		textures.push_back(diffuse);

		material->SetDiffuseTexture(diffuse);

		// Read normal texture if available
		fread(normalTextureName, sizeof(char), 64, file);
		if (strcmp(normalTextureName, "NONE") != 0)
		{
			texturePath = "data/textures/" + std::string(normalTextureName);

			Texture * normal = gTextureManager->RequestTexture(texturePath, vulkan->GetVulkanDevice(), cmdBuffer);
			if (normal == nullptr)
				return false;

			textures.push_back(normal);

			material->SetNormalTexture(normal);
		}

		std::string matName, matTextureName;
		float metallicOffset, roughnessOffset;
		matFile >> matName >> matTextureName >> metallicOffset >> roughnessOffset;
		
		// Read material texture
		if (matTextureName == "NONE")
			texturePath = "data/textures/default_material.rct";
		else
			texturePath = "data/textures/" + matTextureName;

		Texture * matTexture = gTextureManager->RequestTexture(texturePath, vulkan->GetVulkanDevice(), cmdBuffer);
		if (matTexture == nullptr)
			return false;

		textures.push_back(matTexture);

		material->SetMaterialTexture(matTexture);
		material->SetMetallicOffset(metallicOffset);
		material->SetRoughnessOffset(roughnessOffset);

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

	SAFE_UNLOAD(skinnedVS_bone_UBO, vulkanDevice);
	SAFE_UNLOAD(skinnedVS_UBO, vulkanDevice);

	for (unsigned int i = 0; i < textures.size(); i++)
		gTextureManager->ReleaseTexture(textures[i], vulkanDevice);

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
	if(vulkanPipeline->GetPipelineName() == "SKINNED")
		vertexUniformBuffer.MVP = camera->GetProjectionMatrix() * camera->GetViewMatrix() * vertexUniformBuffer.worldMatrix;

	skinnedVS_UBO->Update(vulkan->GetVulkanDevice(), &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		if (vulkanPipeline->GetPipelineName() == "SKINNED")
		{
			meshes[i]->UpdateUniformBuffer(vulkan);
			UpdateDescriptorSet(vulkan, vulkanPipeline, meshes[i], NULL);

			// Record draw command
			drawCmdBuffers[i]->BeginRecordingSecondary(vulkan->GetDeferredRenderpass()->GetRenderpass(), vulkan->GetDeferredFramebuffer());

			vulkan->InitViewportAndScissors(drawCmdBuffers[i], (float)gSettings->GetWindowWidth(), (float)gSettings->GetWindowHeight(),
				(uint32_t)gSettings->GetWindowWidth(), (uint32_t)gSettings->GetWindowHeight());
			vulkanPipeline->SetActive(drawCmdBuffers[i]);
			meshes[i]->Render(vulkan, drawCmdBuffers[i]);

			drawCmdBuffers[i]->EndRecording();
			drawCmdBuffers[i]->ExecuteSecondary(commandBuffer);
		}
		else if (vulkanPipeline->GetPipelineName() == "SHADOWSKINNED")
		{
			UpdateDescriptorSet(vulkan, vulkanPipeline, meshes[i], shadowMaps);

			// Record draw command
			drawCmdBuffers[i]->BeginRecordingSecondary(shadowMaps->GetShadowRenderpass()->GetRenderpass(), shadowMaps->GetFramebuffer());

			vulkan->InitViewportAndScissors(drawCmdBuffers[i], (float)shadowMaps->GetMapSize(), (float)shadowMaps->GetMapSize(),
				shadowMaps->GetMapSize(), shadowMaps->GetMapSize());

			shadowMaps->SetDepthBias(drawCmdBuffers[i]);
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
		currentAnim->Update(gTimer->GetDelta(), boneOffsets, boneMapping);
	
		std::vector<glm::mat4> boneTransforms = currentAnim->GetBoneTransforms();
		memcpy(boneUniformBufferData.bones, boneTransforms.data(), sizeof(glm::mat4) * boneTransforms.size());

		skinnedVS_bone_UBO->Update(vulkan->GetVulkanDevice(), &boneUniformBufferData, sizeof(boneUniformBufferData));
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

void SkinnedModel::UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * pipeline, SkinnedMesh * mesh, ShadowMaps * shadowMaps)
{
	if (pipeline->GetPipelineName() == "SKINNED")
	{
		VkWriteDescriptorSet descriptorWrite[6];

		descriptorWrite[0] = {};
		descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[0].pNext = NULL;
		descriptorWrite[0].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[0].descriptorCount = 1;
		descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[0].pBufferInfo = skinnedVS_UBO->GetBufferInfo();
		descriptorWrite[0].dstArrayElement = 0;
		descriptorWrite[0].dstBinding = 0;

		descriptorWrite[1] = {};
		descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[1].pNext = NULL;
		descriptorWrite[1].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[1].descriptorCount = 1;
		descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[1].pBufferInfo = skinnedVS_bone_UBO->GetBufferInfo();
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

		// Write mesh material texture
		VkDescriptorImageInfo materialTextureDesc{};
		materialTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		materialTextureDesc.imageView = *mesh->GetMaterial()->GetMaterialTexture()->GetImageView();
		materialTextureDesc.sampler = vulkan->GetColorSampler();

		descriptorWrite[3] = {};
		descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[3].pNext = NULL;
		descriptorWrite[3].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[3].descriptorCount = 1;
		descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite[3].pImageInfo = &materialTextureDesc;
		descriptorWrite[3].dstArrayElement = 0;
		descriptorWrite[3].dstBinding = 3;

		// Write mesh normal texture if available
		VkDescriptorImageInfo normalTextureDesc{};
		if (mesh->GetMaterial()->HasNormalMap())
		{
			normalTextureDesc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			normalTextureDesc.imageView = *mesh->GetMaterial()->GetNormalTexture()->GetImageView();
			normalTextureDesc.sampler = vulkan->GetColorSampler();
		}

		descriptorWrite[4] = {};
		descriptorWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[4].pNext = NULL;
		descriptorWrite[4].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[4].descriptorCount = 1;
		descriptorWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite[4].pImageInfo = (mesh->GetMaterial()->HasNormalMap() ? &normalTextureDesc : &diffuseTextureDesc);
		descriptorWrite[4].dstArrayElement = 0;
		descriptorWrite[4].dstBinding = 4;

		// Update material uniform buffer
		descriptorWrite[5] = {};
		descriptorWrite[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[5].pNext = NULL;
		descriptorWrite[5].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[5].descriptorCount = 1;
		descriptorWrite[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[5].pBufferInfo = mesh->GetMaterialBufferInfo();
		descriptorWrite[5].dstArrayElement = 0;
		descriptorWrite[5].dstBinding = 5;

		vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);
	}
	else if (pipeline->GetPipelineName() == "SHADOWSKINNED")
	{
		VkWriteDescriptorSet descriptorWrite[3];

		descriptorWrite[0] = {};
		descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[0].pNext = NULL;
		descriptorWrite[0].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[0].descriptorCount = 1;
		descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[0].pBufferInfo = skinnedVS_UBO->GetBufferInfo();
		descriptorWrite[0].dstArrayElement = 0;
		descriptorWrite[0].dstBinding = 0;

		descriptorWrite[1] = {};
		descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[1].pNext = NULL;
		descriptorWrite[1].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[1].descriptorCount = 1;
		descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[1].pBufferInfo = skinnedVS_bone_UBO->GetBufferInfo();
		descriptorWrite[1].dstArrayElement = 0;
		descriptorWrite[1].dstBinding = 1;

		descriptorWrite[2] = {};
		descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite[2].pNext = NULL;
		descriptorWrite[2].dstSet = pipeline->GetDescriptorSet();
		descriptorWrite[2].descriptorCount = 1;
		descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite[2].pBufferInfo = shadowMaps->GetBufferInfo();
		descriptorWrite[2].dstArrayElement = 0;
		descriptorWrite[2].dstBinding = 2;

		vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);
	}
}