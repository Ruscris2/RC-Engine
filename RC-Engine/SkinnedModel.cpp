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

bool SkinnedModel::Init(std::string filename, VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VulkanCommandBuffer * cmdBuffer)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VkResult result;

	VkMemoryAllocateInfo allocInfo{};
	uint8_t *pData;

	// Uniform buffer init
	vertexUniformBuffer.worldMatrix = glm::mat4(1.0f);
	vertexUniformBuffer.MVP = glm::mat4();
	for (unsigned int i = 0; i < MAX_BONES; i++)
		vertexUniformBuffer.bones[i] = glm::mat4();

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
		if (!mesh->Init(vulkan, file, vulkanPipeline, vsUniformBufferInfo))
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

		// Write descriptor set for each mesh
		meshes[i]->WriteDescriptorSet(vulkan, vsUniformBufferInfo);

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

void SkinnedModel::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera)
{
	currentAnim->Update(gTimer->GetDelta(), boneOffsets, boneMapping);

	uint8_t *pData;

	// Update vertex uniform buffer
	vertexUniformBuffer.MVP = vulkan->GetProjectionMatrix() * camera->GetViewMatrix() * vertexUniformBuffer.worldMatrix;

	if (currentAnim != NULL)
	{
		std::vector<glm::mat4> boneTransforms = currentAnim->GetBoneTransforms();
		memcpy(vertexUniformBuffer.bones, boneTransforms.data(), sizeof(glm::mat4) * boneTransforms.size());
	}

	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory);

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->UpdateUniformBuffer(vulkan);

		// Record draw command
		drawCmdBuffers[i]->BeginRecordingSecondary(vulkan->GetDeferredRenderpass()->GetRenderpass(), vulkan->GetDeferredFramebuffer());

		vulkan->InitViewportAndScissors(drawCmdBuffers[i]);
		vulkanPipeline->SetActive(drawCmdBuffers[i]);
		vkCmdBindDescriptorSets(drawCmdBuffers[i]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetPipelineLayout(), 0, 1, meshes[i]->GetDescriptorSet(), 0, NULL);
		meshes[i]->Render(vulkan, drawCmdBuffers[i]);

		drawCmdBuffers[i]->EndRecording();
		drawCmdBuffers[i]->ExecuteSecondary(commandBuffer);
	}
}

void SkinnedModel::SetPosition(float x, float y, float z)
{
	posX = x;
	posY = y;
	posZ = z;
	UpdateWorldMatrix();
}

void SkinnedModel::SetRotation(float x, float y, float z)
{
	rotX = x;
	rotY = y;
	rotZ = z;
	UpdateWorldMatrix();
}

void SkinnedModel::SetAnimation(Animation * anim)
{
	currentAnim = anim;
}

void SkinnedModel::UpdateWorldMatrix()
{
	vertexUniformBuffer.worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, posZ));
	vertexUniformBuffer.worldMatrix = glm::rotate(vertexUniformBuffer.worldMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
	vertexUniformBuffer.worldMatrix = glm::rotate(vertexUniformBuffer.worldMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
	vertexUniformBuffer.worldMatrix = glm::rotate(vertexUniformBuffer.worldMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
}