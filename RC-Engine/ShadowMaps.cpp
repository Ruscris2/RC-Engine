/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: ShadowMaps.cpp                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "ShadowMaps.h"
#include "LogManager.h"
#include "StdInc.h"
#include "VulkanTools.h"
#include "Input.h"

extern LogManager * gLogManager;
extern Input * gInput;

ShadowMaps::ShadowMaps()
{
	depthAttachment = NULL;
	renderpass = NULL;
	shadowGS_UBO = NULL;
}

bool ShadowMaps::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, Camera * camera)
{
	VkResult result;

	mapSize = 2048;

	// Create framebuffer attachments
	depthAttachment = new FrameBufferAttachment();
	if (!depthAttachment->Create(vulkan->GetVulkanDevice(), vulkan->GetDepthAttachment()->GetFormat(),
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, cmdBuffer, mapSize, mapSize, SHADOW_CASCADE_COUNT))
	{
		gLogManager->AddMessage("ERROR: Failed to create depth framebuffer attachment!");
		return false;
	}

	// Create the renderpass
	VkAttachmentDescription attachmentDesc{};
	VkAttachmentReference attachmentRef;

	attachmentDesc.format = vulkan->GetDepthAttachment()->GetFormat();
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.flags = 0;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	attachmentRef.attachment = 0;
	attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VulkanRenderpassCI renderpassCI;
	renderpassCI.attachments = &attachmentDesc;
	renderpassCI.attachmentCount = 1;
	renderpassCI.attachmentRefs = VK_NULL_HANDLE;
	renderpassCI.depthAttachmentRef = &attachmentRef;
	renderpassCI.dependencies = VK_NULL_HANDLE;
	renderpassCI.dependenciesCount = 0;

	renderpass = new VulkanRenderpass();
	if (!renderpass->Init(vulkan->GetVulkanDevice(), &renderpassCI))
	{
		gLogManager->AddMessage("ERROR: Failed to create shadow map renderpass!");
		return false;
	}

	// Create the framebuffer
	VkFramebufferCreateInfo fbCI{};
	fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCI.renderPass = renderpass->GetRenderpass();
	fbCI.pAttachments = depthAttachment->GetImageView();
	fbCI.attachmentCount = 1;
	fbCI.width = mapSize;
	fbCI.height = mapSize;
	fbCI.layers = SHADOW_CASCADE_COUNT;

	result = vkCreateFramebuffer(vulkan->GetVulkanDevice()->GetDevice(), &fbCI, VK_NULL_HANDLE, &framebuffer);
	if (result != VK_SUCCESS)
		return false;

	// Create the sampler
	VkSamplerCreateInfo samplerCI{};
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 1.0f;
	samplerCI.maxAnisotropy = 0.0f;
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	result = vkCreateSampler(vulkan->GetVulkanDevice()->GetDevice(), &samplerCI, VK_NULL_HANDLE, &sampler);
	if (result != VK_SUCCESS)
		return false;

	orthoMatrices = new glm::mat4[SHADOW_CASCADE_COUNT];
	viewMatrices = new glm::mat4[SHADOW_CASCADE_COUNT];

	// Projection matrices
	projectionMatrixPartitions = new glm::mat4[SHADOW_CASCADE_COUNT];

	projectionMatrixPartitions[0] = glm::perspective(camera->GetFieldOfView(), camera->GetAspectRatio(), camera->GetNearClip(), 5.0f);
	projectionMatrixPartitions[1] = glm::perspective(camera->GetFieldOfView(), camera->GetAspectRatio(), 5.0f, 15.0f);
	projectionMatrixPartitions[2] = glm::perspective(camera->GetFieldOfView(), camera->GetAspectRatio(), 15.0f, 50.0f);

	// Create geometry shader uniform buffer
	shadowGS_UBO = new VulkanBuffer();
	if (!shadowGS_UBO->Init(vulkan->GetVulkanDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &geometryUniformBuffer,
		sizeof(geometryUniformBuffer), false))
		return false;

	return true;
}

void ShadowMaps::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(shadowGS_UBO, vulkan->GetVulkanDevice());
	SAFE_DELETE(projectionMatrixPartitions);
	SAFE_DELETE(viewMatrices);
	SAFE_DELETE(orthoMatrices);
	vkDestroySampler(vulkan->GetVulkanDevice()->GetDevice(), sampler, VK_NULL_HANDLE);
	vkDestroyFramebuffer(vulkan->GetVulkanDevice()->GetDevice(), framebuffer, VK_NULL_HANDLE);
	SAFE_UNLOAD(renderpass, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(depthAttachment, vulkan->GetVulkanDevice());
}

void ShadowMaps::BeginShadowPass(VulkanCommandBuffer * commandBuffer)
{
	commandBuffer->BeginRecording();

	renderpass->BeginRenderpass(commandBuffer, 0.0f, 0.0f, 0.0f, 0.0f, framebuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
		mapSize, mapSize);
}

void ShadowMaps::EndShadowPass(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer)
{
	renderpass->EndRenderpass(commandBuffer);

	commandBuffer->EndRecording();

	commandBuffer->Execute(vulkanDevice, NULL, NULL, NULL, true);
}

void ShadowMaps::SetDepthBias(VulkanCommandBuffer * cmdBuffer)
{
	vkCmdSetDepthBias(cmdBuffer->GetCommandBuffer(), 0.001f, 0.0f, 1.0f);
}

void ShadowMaps::UpdatePartitions(VulkanInterface * vulkan, Camera * viewcamera, Light * light)
{
	if (gInput->IsKeyPressed(KEYBOARD_KEY_Q))
		return;

	for (int i = 0; i < SHADOW_CASCADE_COUNT; i++)
	{
		glm::vec3 frustumCorners[8] =
		{
			glm::vec3(-1.0f, 1.0f, -1.0f), // top left near 0
			glm::vec3(1.0f, 1.0f, -1.0f), // top right near 1
			glm::vec3(1.0f, -1.0f, -1.0f), // bottom right near 2
			glm::vec3(-1.0f, -1.0f, -1.0f), // bottom left near 3
			glm::vec3(-1.0f, 1.0f, 1.0f), // top left far 4
			glm::vec3(1.0f, 1.0f, 1.0f), // top right far 5
			glm::vec3(1.0f, -1.0f, 1.0f), // bottom right far 6
			glm::vec3(-1.0f, -1.0f, 1.0f) // bottom left far 7
		};

		// Transform partition into light space
		glm::mat4 viewProjMatrix = projectionMatrixPartitions[i] * viewcamera->GetViewMatrix();
		viewProjMatrix = glm::inverse(viewProjMatrix);

		// Transorm the coords of the cube into the viewProj frustum
		for (int j = 0; j < 8; j++)
			frustumCorners[j] = VulkanTools::Vec3Transform(frustumCorners[j], viewProjMatrix);

		// Calculate the radius
		float radius = glm::distance(frustumCorners[0], frustumCorners[6]) / 2.0f;

		// Calculate the center
		glm::vec3 frustumCenter = glm::vec3(0.0f, 0.0f, 0.0f);

		for (int j = 0; j < 8; j++)
			frustumCenter += frustumCorners[j];
		frustumCenter /= 8.0f;

		// Texel snapping
		float texelsPerUnit = (float)mapSize / (radius * 4.0f);

		glm::mat4 scalar = glm::scale(glm::mat4(), glm::vec3(texelsPerUnit, texelsPerUnit, texelsPerUnit));

		glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 lookAt, lookAtInv;
		glm::vec3 baseLookAt = -light->GetLightDirection();

		lookAt = glm::lookAt(zero, baseLookAt, up);
		lookAt = scalar * lookAt;
		lookAtInv = glm::inverse(lookAt);

		frustumCenter = VulkanTools::Vec3Transform(frustumCenter, lookAt);
		frustumCenter.x = glm::floor(frustumCenter.x);
		frustumCenter.y = glm::floor(frustumCenter.y);
		frustumCenter = VulkanTools::Vec3Transform(frustumCenter, lookAtInv);
		
		glm::vec3 eye = frustumCenter - (light->GetLightDirection() * radius * 2.0f);

		// Create the view matrix and projection matrix
		viewMatrices[i] = glm::lookAt(eye, frustumCenter, up);
		orthoMatrices[i] = glm::ortho(-radius * 2.0f, radius * 2.0f, -radius * 2.0f, radius * 2.0f, -radius * 6.0f, radius * 6.0f);
		geometryUniformBuffer.lightViewProj[i] = orthoMatrices[i] * viewMatrices[i];
	}

	shadowGS_UBO->Update(vulkan->GetVulkanDevice(), &geometryUniformBuffer, sizeof(geometryUniformBuffer));
}

VulkanRenderpass * ShadowMaps::GetShadowRenderpass()
{
	return renderpass;
}

VkFramebuffer ShadowMaps::GetFramebuffer()
{
	return framebuffer;
}

VkImageView * ShadowMaps::GetImageView()
{
	return depthAttachment->GetImageView();
}

VkDescriptorBufferInfo * ShadowMaps::GetBufferInfo()
{
	return shadowGS_UBO->GetBufferInfo();
}

glm::mat4 ShadowMaps::GetLightViewProj(int index)
{
	return geometryUniformBuffer.lightViewProj[index];
}

VkSampler ShadowMaps::GetSampler()
{
	return sampler;
}

uint32_t ShadowMaps::GetMapSize()
{
	return mapSize;
}
