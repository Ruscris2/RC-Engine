/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "SceneManager.h"
#include "StdInc.h"
#include "LogManager.h"
#include "Input.h"

extern LogManager * gLogManager;
extern Input * gInput;

SceneManager::SceneManager()
{
	camera = NULL;
	light = NULL;
	deferredCommandBuffer = NULL;
	renderCommandBuffer = NULL;
	defaultShader = NULL;
	deferredShader = NULL;
	defaultPipeline = NULL;
	deferredPipeline = NULL;
	defaultShaderCanvas = NULL;
	model = NULL;
	model2 = NULL;
}

SceneManager::~SceneManager()
{
	SAFE_DELETE(light);
	SAFE_DELETE(camera);
}

bool SceneManager::Init(VulkanInterface * vulkan)
{
	camera = new Camera();
	camera->Init();
	
	light = new Light();

	deferredCommandBuffer = new VulkanCommandBuffer();
	if (!deferredCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool()))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (deferredCommandBuffer)");
		return false;
	}

	renderCommandBuffer = new VulkanCommandBuffer();
	if (!renderCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool()))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (renderCommandBuffer)");
		return false;
	}

	defaultShader = new DefaultShader();
	if (!defaultShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init shader!");
		return false;
	}

	deferredShader = new DeferredShader();
	if (!deferredShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init deferred shader!");
		return false;
	}

	if (!BuildDefaultPipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init default pipeline!");
		return false;
	}

	if (!BuildDeferredPipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init deferred pipeline!");
		return false;
	}

	defaultShaderCanvas = new Canvas();
	if (!defaultShaderCanvas->Init(vulkan, defaultPipeline, vulkan->GetPositionAttachment()->GetImageView(), vulkan->GetNormalAttachment()->GetImageView(),
		vulkan->GetAlbedoAttachment()->GetImageView()))
	{
		gLogManager->AddMessage("ERROR: Failed to init default shader canvas!");
		return false;
	}

	model = new Model();
	if (!model->Init("data/models/teapot.rcm", vulkan, deferredPipeline, renderCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to init model!");
		return false;
	}
	model->SetPosition(-2.0f, 0.0f, 0.0f);

	model2 = new Model();
	if (!model2->Init("data/models/teapot.rcm", vulkan, deferredPipeline, renderCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to init model2!");
		return false;
	}
	model2->SetPosition(2.0f, 0.0f, 0.0f);
	model2->SetRotation(0.0f, 135.0f, -30.0f);

	return true;
}

void SceneManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(defaultShaderCanvas, vulkan);
	SAFE_UNLOAD(deferredPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(defaultPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(deferredShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(defaultShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(model2, vulkan);
	SAFE_UNLOAD(model, vulkan);
	SAFE_UNLOAD(renderCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(deferredCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
}

int imageIndex = 4;
void SceneManager::Render(VulkanInterface * vulkan)
{
	camera->HandleInput();

	// Debug deferred shading
	if (gInput->WasKeyPressed(KEYBOARD_KEY_1))
		imageIndex = 1;
	if (gInput->WasKeyPressed(KEYBOARD_KEY_2))
		imageIndex = 2;
	if (gInput->WasKeyPressed(KEYBOARD_KEY_3))
		imageIndex = 3;
	if (gInput->WasKeyPressed(KEYBOARD_KEY_4))
		imageIndex = 4;

	vulkan->BeginScene3D(deferredCommandBuffer);

	deferredPipeline->SetActive(deferredCommandBuffer);
	
	model->Render(vulkan, deferredCommandBuffer, deferredPipeline, camera);
	model2->Render(vulkan, deferredCommandBuffer, deferredPipeline, camera);

	vulkan->EndScene3D(deferredCommandBuffer);
	
	vulkan->BeginScene2D(renderCommandBuffer, defaultPipeline);

	defaultPipeline->SetActive(renderCommandBuffer);
	defaultShaderCanvas->Render(vulkan, renderCommandBuffer, defaultPipeline, vulkan->GetOrthoMatrix(), light, imageIndex);

	vulkan->EndScene2D(renderCommandBuffer);
}

bool SceneManager::BuildDefaultPipeline(VulkanInterface * vulkan)
{
	VkVertexInputAttributeDescription vertexLayoutDefault[2];

	vertexLayoutDefault[0].binding = 0;
	vertexLayoutDefault[0].location = 0;
	vertexLayoutDefault[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutDefault[0].offset = 0;

	vertexLayoutDefault[1].binding = 0;
	vertexLayoutDefault[1].location = 1;
	vertexLayoutDefault[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexLayoutDefault[1].offset = sizeof(float) * 3;

	VkDescriptorSetLayoutBinding layoutBindingsDefault[5];

	// Vertex shader uniform buffer
	layoutBindingsDefault[0].binding = 0;
	layoutBindingsDefault[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDefault[0].descriptorCount = 1;
	layoutBindingsDefault[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsDefault[0].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[1].binding = 1;
	layoutBindingsDefault[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[1].descriptorCount = 1;
	layoutBindingsDefault[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[1].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[2].binding = 2;
	layoutBindingsDefault[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[2].descriptorCount = 1;
	layoutBindingsDefault[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[2].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[3].binding = 3;
	layoutBindingsDefault[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[3].descriptorCount = 1;
	layoutBindingsDefault[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[3].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[4].binding = 4;
	layoutBindingsDefault[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDefault[4].descriptorCount = 1;
	layoutBindingsDefault[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[4].pImmutableSamplers = VK_NULL_HANDLE;

	struct DefaultVertex {
		float x, y, z;
		float u, v;
	};

	defaultPipeline = new VulkanPipeline();
	if (!defaultPipeline->Init(vulkan->GetVulkanDevice(), defaultShader, vulkan->GetMainRenderpass(), vertexLayoutDefault, 2,
		layoutBindingsDefault, 5, sizeof(DefaultVertex), 1))
		return false;

	return true;
}

bool SceneManager::BuildDeferredPipeline(VulkanInterface * vulkan)
{
	VkVertexInputAttributeDescription vertexLayoutDeferred[3];

	vertexLayoutDeferred[0].binding = 0;
	vertexLayoutDeferred[0].location = 0;
	vertexLayoutDeferred[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutDeferred[0].offset = 0;

	vertexLayoutDeferred[1].binding = 0;
	vertexLayoutDeferred[1].location = 1;
	vertexLayoutDeferred[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexLayoutDeferred[1].offset = sizeof(float) * 3;

	vertexLayoutDeferred[2].binding = 0;
	vertexLayoutDeferred[2].location = 2;
	vertexLayoutDeferred[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutDeferred[2].offset = sizeof(float) * 5;

	VkDescriptorSetLayoutBinding layoutBindingsDeferred[2];

	// Vertex shader uniform buffer
	layoutBindingsDeferred[0].binding = 0;
	layoutBindingsDeferred[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDeferred[0].descriptorCount = 1;
	layoutBindingsDeferred[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsDeferred[0].pImmutableSamplers = VK_NULL_HANDLE;

	// Fragment shader sampler
	layoutBindingsDeferred[1].binding = 1;
	layoutBindingsDeferred[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDeferred[1].descriptorCount = 1;
	layoutBindingsDeferred[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDeferred[1].pImmutableSamplers = VK_NULL_HANDLE;

	struct DeferredVertex {
		float x, y, z;
		float u, v;
		float nx, ny, nz;
	};

	deferredPipeline = new VulkanPipeline();
	if (!deferredPipeline->Init(vulkan->GetVulkanDevice(), deferredShader, vulkan->GetDeferredRenderpass(), vertexLayoutDeferred, 3,
		layoutBindingsDeferred, 2, sizeof(DeferredVertex), 3))
		return false;

	return true;
}