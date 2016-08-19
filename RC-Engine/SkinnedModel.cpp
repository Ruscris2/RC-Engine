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

extern LogManager * gLogManager;
extern Timer * gTimer;
extern Settings * gSettings;

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

	SAFE_UNLOAD(skinnedVS_bone_UBO, vulkanDevice);
	SAFE_UNLOAD(skinnedVS_UBO, vulkanDevice);

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
	if(vulkanPipeline->GetPipelineName() == "SKINNED")
		vertexUniformBuffer.MVP = vulkan->GetProjectionMatrix() * camera->GetViewMatrix() * vertexUniformBuffer.worldMatrix;
	else if(vulkanPipeline->GetPipelineName() == "SHADOWSKINNED")
		vertexUniformBuffer.MVP = shadowMaps->GetOrthoMatrix() * shadowMaps->GetViewMatrix() * vertexUniformBuffer.worldMatrix;

	skinnedVS_UBO->Update(vulkan->GetVulkanDevice(), &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		if (vulkanPipeline->GetPipelineName() == "SKINNED")
		{
			meshes[i]->UpdateUniformBuffer(vulkan);
			UpdateDescriptorSet(vulkan, vulkanPipeline, meshes[i]);

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
			UpdateDescriptorSet(vulkan, vulkanPipeline, meshes[i]);

			// Record draw command
			drawCmdBuffers[i]->BeginRecordingSecondary(shadowMaps->GetShadowRenderpass()->GetRenderpass(), shadowMaps->GetFramebuffer());

			vulkan->InitViewportAndScissors(drawCmdBuffers[i], (float)shadowMaps->GetMapWidth(), (float)shadowMaps->GetMapHeight(),
				shadowMaps->GetMapWidth(), shadowMaps->GetMapHeight());
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

		vkUpdateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), sizeof(descriptorWrite) / sizeof(descriptorWrite[0]), descriptorWrite, 0, NULL);
	}
}