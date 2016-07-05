/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "SceneManager.h"
#include "StdInc.h"
#include "LogManager.h"

extern LogManager * gLogManager;

SceneManager::SceneManager()
{
	camera = NULL;
	light = NULL;
	renderCommandBuffer = NULL;
	vulkanShader = NULL;
	vulkanPipeline = NULL;
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

	renderCommandBuffer = new VulkanCommandBuffer();
	if (!renderCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool()))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (renderCommandBuffer)");
		return false;
	}

	vulkanShader = new VulkanShader();
	if (!vulkanShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init shader!");
		return false;
	}

	vulkanPipeline = new VulkanPipeline();
	if (!vulkanPipeline->Init(vulkan->GetVulkanDevice(), vulkanShader, vulkan->GetVulkanSwapchain()))
	{
		gLogManager->AddMessage("ERROR: Failed to init pipeline!");
		return false;
	}

	texture = new Texture();
	if (!texture->Init(vulkan, "data/textures/test.rct"))
	{
		gLogManager->AddMessage("ERROR: Couldn't init texture!");
		return false;
	}

	model = new Model();
	if (!model->Init("data/models/teapot.rcm", vulkan, vulkanPipeline, texture, light))
	{
		gLogManager->AddMessage("ERROR: Failed to init model!");
		return false;
	}
	model->SetPosition(-2.0f, 0.0f, 0.0f);

	model2 = new Model();
	if (!model2->Init("data/models/teapot.rcm", vulkan, vulkanPipeline, texture, light))
	{
		gLogManager->AddMessage("ERROR: Failed to init model2!");
		return false;
	}
	model2->SetPosition(2.0f, 0.0f, 0.0f);

	return true;
}

void SceneManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(texture, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(vulkanPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(vulkanShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(model2, vulkan);
	SAFE_UNLOAD(model, vulkan);
	SAFE_UNLOAD(renderCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
}

void SceneManager::Render(VulkanInterface * vulkan)
{
	camera->HandleInput();

	vulkan->BeginScene(renderCommandBuffer);

	vulkanPipeline->SetActive(renderCommandBuffer);
	
	model->Render(vulkan, renderCommandBuffer, vulkanPipeline, camera, light);
	model2->Render(vulkan, renderCommandBuffer, vulkanPipeline, camera, light);

	vulkan->EndScene(renderCommandBuffer);
}
