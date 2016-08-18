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

	render = false;
}

bool ShadowMaps::Init(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer)
{
	VkResult result;

	mapWidth = mapHeight = 1024;

	// Create framebuffer attachments
	depthAttachment = new FrameBufferAttachment();
	if (!depthAttachment->Create(vulkan->GetVulkanDevice(), vulkan->GetDepthAttachment()->GetFormat(),
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, cmdBuffer, mapWidth, mapHeight))
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
	fbCI.width = mapWidth;
	fbCI.height = mapHeight;
	fbCI.layers = 1;

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

	orthoMatrices = new glm::mat4[vulkan->GetProjectionMatrixPartitionCount()];
	viewMatrices = new glm::mat4[vulkan->GetProjectionMatrixPartitionCount()];

	return true;
}

void ShadowMaps::Unload(VulkanInterface * vulkan)
{
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
		mapWidth, mapHeight);
}

void ShadowMaps::EndShadowPass(VulkanDevice * vulkanDevice, VulkanCommandBuffer * commandBuffer)
{
	renderpass->EndRenderpass(commandBuffer);

	commandBuffer->EndRecording();

	commandBuffer->Execute(vulkanDevice, NULL, NULL, NULL, true);
}

void ShadowMaps::SetDepthBias(VulkanCommandBuffer * cmdBuffer)
{
	vkCmdSetDepthBias(cmdBuffer->GetCommandBuffer(), 1.25f, 0.0f, 1.75f);
}

void ShadowMaps::UpdatePartitions(VulkanInterface * vulkan, Camera * viewcamera, Light * light)
{
	for (int i = 0; i < vulkan->GetProjectionMatrixPartitionCount(); i++)
	{
		if (gInput->WasKeyPressed(KEYBOARD_KEY_Q))
		{
			char msg[32];
			sprintf(msg, "CASCADE %d", i);
			gLogManager->AddMessage(msg);
		}

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

		glm::mat4 viewProjMatrix = vulkan->GetProjectionMatrixPartition(i) * viewcamera->GetViewMatrix();
		viewProjMatrix = glm::inverse(viewProjMatrix);

		for (int j = 0; j < 8; j++)
		{
			frustumCorners[j] = VulkanTools::Vec3Transform(frustumCorners[j], viewProjMatrix);
			if (gInput->WasKeyPressed(KEYBOARD_KEY_Q))
			{
				gLogManager->PrintVector(frustumCorners[j]);

				WireframeModel * ptrFace1;
				WireframeModel * ptrFace2;
				WireframeModel * ptrFace3;
				WireframeModel * ptrFace4;
				glm::vec4 color;

				if (i == 0)
				{
					ptrFace1 = debugCascade1Face1;
					ptrFace2 = debugCascade1Face2;
					ptrFace3 = debugCascade1Face3;
					ptrFace4 = debugCascade1Face4;
					color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				}
				else if (i == 1)
				{
					ptrFace1 = debugCascade2Face1;
					ptrFace2 = debugCascade2Face2;
					ptrFace3 = debugCascade2Face3;
					ptrFace4 = debugCascade2Face4;
					color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
				}
				else
				{
					ptrFace1 = debugCascade3Face1;
					ptrFace2 = debugCascade3Face2;
					ptrFace3 = debugCascade3Face3;
					ptrFace4 = debugCascade3Face4;
					color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
				}

				if (ptrFace1 != NULL) SAFE_UNLOAD(ptrFace1, vulkan);
				if (ptrFace2 != NULL) SAFE_UNLOAD(ptrFace2, vulkan);
				if (ptrFace3 != NULL) SAFE_UNLOAD(ptrFace3, vulkan);
				if (ptrFace4 != NULL) SAFE_UNLOAD(ptrFace4, vulkan);

				GEOMETRY_GENERATE_INFO info;
				info.type = GEOMETRY_TYPE_POLYGON;
				info.v1 = frustumCorners[1];
				info.v2 = frustumCorners[5];
				info.v3 = frustumCorners[6];
				info.v4 = frustumCorners[2];

				ptrFace1 = new WireframeModel();
				ptrFace1->Init(vulkan, info, color);
				ptrFace1->SetPosition(0.0f, 0.0f, 0.0f);

				info.v1 = frustumCorners[0];
				info.v2 = frustumCorners[4];
				info.v3 = frustumCorners[5];
				info.v4 = frustumCorners[1];

				ptrFace2 = new WireframeModel();
				ptrFace2->Init(vulkan, info, color);
				ptrFace2->SetPosition(0.0f, 0.0f, 0.0f);

				info.v1 = frustumCorners[0];
				info.v2 = frustumCorners[4];
				info.v3 = frustumCorners[7];
				info.v4 = frustumCorners[3];

				ptrFace3 = new WireframeModel();
				ptrFace3->Init(vulkan, info, color);
				ptrFace3->SetPosition(0.0f, 0.0f, 0.0f);

				info.v1 = frustumCorners[3];
				info.v2 = frustumCorners[2];
				info.v3 = frustumCorners[6];
				info.v4 = frustumCorners[7];

				ptrFace4 = new WireframeModel();
				ptrFace4->Init(vulkan, info, color);
				ptrFace4->SetPosition(0.0f, 0.0f, 0.0f);
				render = true;

				if (i == 0)
				{
					debugCascade1Face1 = ptrFace1;
					debugCascade1Face2 = ptrFace2;
					debugCascade1Face3 = ptrFace3;
					debugCascade1Face4 = ptrFace4;
				}
				else if (i == 1)
				{
					debugCascade2Face1 = ptrFace1;
					debugCascade2Face2 = ptrFace2;
					debugCascade2Face3 = ptrFace3;
					debugCascade2Face4 = ptrFace4;
				}
				else
				{
					debugCascade3Face1 = ptrFace1;
					debugCascade3Face2 = ptrFace2;
					debugCascade3Face3 = ptrFace3;
					debugCascade3Face4 = ptrFace4;
				}
			}
		}

		float radius = glm::distance(frustumCorners[0], frustumCorners[6]) / 2.0f;

		if (gInput->WasKeyPressed(KEYBOARD_KEY_Q))
		{
			char msg[32];
			sprintf(msg, "RADIUS %f", radius);
			gLogManager->AddMessage(msg);
		}

		glm::vec3 frustumCenter = glm::vec3(0.0f, 0.0f, 0.0f);

		for (int j = 0; j < 8; j++)
			frustumCenter += frustumCorners[j];
		frustumCenter /= 8.0f;

		float texelsPerUnit = (float)mapWidth / (radius * 2.0f);

		glm::mat4 scalar = glm::scale(glm::mat4(), glm::vec3(texelsPerUnit, texelsPerUnit, texelsPerUnit));

		glm::vec3 zero = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 lookAt, lookAtInv;
		glm::vec3 baseLookAt = -light->GetLightDirection();

		lookAt = glm::lookAt(zero, baseLookAt, up);
		lookAt *= scalar;
		lookAtInv = glm::inverse(lookAt);

		frustumCenter = VulkanTools::Vec3Transform(frustumCenter, lookAt);
		frustumCenter.x = glm::floor(frustumCenter.x);
		frustumCenter.y = glm::floor(frustumCenter.y);
		frustumCenter = VulkanTools::Vec3Transform(frustumCenter, lookAtInv);

		glm::vec3 eye = frustumCenter - (light->GetLightDirection() * radius * 2.0f);

		viewMatrices[i] = glm::lookAt(eye, frustumCenter, up);
		orthoMatrices[i] = glm::ortho(-radius, radius, -radius, radius, -radius * 6.0f, radius * 6.0f);
	}
}

void ShadowMaps::RenderDebug(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, VulkanPipeline * pipeline, Camera * camera, int framebufferId)
{
	if (render)
	{
		debugCascade1Face1->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade1Face2->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade1Face3->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade1Face4->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);

		debugCascade2Face1->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade2Face2->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade2Face3->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade2Face4->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);

		debugCascade3Face1->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade3Face2->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade3Face3->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
		debugCascade3Face4->Render(vulkan, cmdBuffer, pipeline, camera, framebufferId);
	}
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

glm::mat4 ShadowMaps::GetViewMatrix()
{
	return viewMatrices[2];
}

glm::mat4 ShadowMaps::GetOrthoMatrix()
{
	return orthoMatrices[2];
}

VkSampler ShadowMaps::GetSampler()
{
	return sampler;
}

uint32_t ShadowMaps::GetMapWidth()
{
	return mapWidth;
}

uint32_t ShadowMaps::GetMapHeight()
{
	return mapHeight;
}
