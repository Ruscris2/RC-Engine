/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>

#include "SceneManager.h"
#include "StdInc.h"
#include "LogManager.h"
#include "Input.h"
#include "Timer.h"
#include "StdInc.h"

extern LogManager * gLogManager;
extern Input * gInput;
extern Timer * gTimer;

SceneManager::SceneManager()
{
	lastGameState = currentGameState = GAME_STATE_UNINITIALIZED;

	physics = NULL;
	camera = NULL;
	light = NULL;
	pipelineManager = NULL;

	initCommandBuffer = NULL;
	deferredCommandBuffer = NULL;

	renderDummy = NULL;
	skydome = NULL;

	idleAnim = NULL;
	walkAnim = NULL;
	fallAnim = NULL;
	jumpAnim = NULL;
	runAnim = NULL;

	player = NULL;

	splashScreen = NULL;
	showSplashScreen = true;

	debugCameraShadowMap = NULL;
}

SceneManager::~SceneManager()
{
	SAFE_DELETE(player);

	SAFE_DELETE(runAnim);
	SAFE_DELETE(jumpAnim);
	SAFE_DELETE(fallAnim);
	SAFE_DELETE(walkAnim);
	SAFE_DELETE(idleAnim);

	SAFE_DELETE(light);
	SAFE_DELETE(camera);
	SAFE_DELETE(physics);
}

bool SceneManager::Init(VulkanInterface * vulkan)
{
	// Init command buffers
	initCommandBuffer = new VulkanCommandBuffer();
	if (!initCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (initCommandBuffer)");
		return false;
	}

	for(size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
		if (!cmdBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
		{
			gLogManager->AddMessage("ERROR: Failed to create a command buffer! (renderCommandBuffers)");
			return false;
		}
		renderCommandBuffers.push_back(cmdBuffer);
	}

	deferredCommandBuffer = new VulkanCommandBuffer();
	if (!deferredCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (deferredCommandBuffer)");
		return false;
	}

	// Init pipeline manager
	pipelineManager = new PipelineManager();
	if (!pipelineManager->InitUIPipelines(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init UI pipelines!");
		return false;
	}

	// Init GUI manager
	guiManager = new GUIManager();
	if (!guiManager->Init(vulkan, initCommandBuffer))
		return false;

	// Splash screen logo
	splashScreen = new GUIElement();
	if (!splashScreen->Init(vulkan, initCommandBuffer, "data/textures/logo.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init splash screen logo!");
		return false;
	}
	splashScreen->SetDimensions(0.4f, 0.5f);
	splashScreen->SetPosition(0.3f, 0.25f);

	splashScreenTimer = new GameplayTimer();

	ChangeGameState(GAME_STATE_SPLASH_SCREEN);
	return true;
}

bool SceneManager::LoadGame(VulkanInterface * vulkan)
{
	// Init shadow maps
	shadowMaps = new ShadowMaps();
	if (!shadowMaps->Init(vulkan, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to init shadow maps!");
		return false;
	}

	// Physics init
	physics = new Physics();
	physics->Init();

	// Camera setup
	camera = new Camera();
	camera->Init();
	camera->SetPosition(0.0f, 5.0f, -10.0f);
	camera->SetDirection(0.0f, 0.0f, 1.0f);
	camera->SetCameraState(CAMERA_STATE_ORBIT_PLAYER);

	// Light setup
	light = new Light();
	light->SetAmbientColor(0.45f, 0.45f, 0.5f, 1.0f);
	light->SetDiffuseColor(0.6f, 0.6f, 0.6f, 1.0f);
	light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	light->SetLightDirection(-0.5f, -0.5f, 1.0f);

	if (!pipelineManager->InitGamePipelines(vulkan, shadowMaps))
	{
		gLogManager->AddMessage("ERROR: Failed to init game pipelines!");
		return false;
	}

	// Init render dummy
	renderDummy = new RenderDummy();
	if (!renderDummy->Init(vulkan, pipelineManager->GetDefault(), vulkan->GetPositionAttachment()->GetImageView(), vulkan->GetNormalAttachment()->GetImageView(),
		vulkan->GetAlbedoAttachment()->GetImageView(), vulkan->GetMaterialAttachment()->GetImageView(), vulkan->GetDepthAttachment()->GetImageView(),
		shadowMaps))
	{
		gLogManager->AddMessage("ERROR: Failed to init render dummy!");
		return false;
	}

	// Init skydome
	skydome = new Skydome();
	if (!skydome->Init(vulkan, pipelineManager->GetSkydome()))
	{
		gLogManager->AddMessage("ERROR: Failed to init skydome!");
		return false;
	}
	skydome->SetSkyColor(0.42f, 0.7f, 1.0f, 1.0f);
	skydome->SetAtmosphereColor(1.0f, 1.0f, 0.94f, 1.0f);
	skydome->SetGroundColor(0.2f, 0.2f, 0.2f, 1.0f);
	skydome->SetAtmosphereHeight(0.2f);

	// Load map files
	if (!LoadMapFile("data/testmap.map", vulkan))
		return false;

	// Skinned models and animations
	male = new SkinnedModel();
	if (!male->Init("data/models/male.rcs", vulkan, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to init male model!");
		return false;
	}

	// Animations
	idleAnim = new Animation();
	if (!idleAnim->Init("data/anims/idle.fbx", 52, true))
		return false;
	idleAnim->SetAnimationSpeed(0.0005f);

	walkAnim = new Animation();
	if (!walkAnim->Init("data/anims/walk.fbx", 52, true))
		return false;

	fallAnim = new Animation();
	if (!fallAnim->Init("data/anims/falling.fbx", 52, true))
		return false;
	fallAnim->SetAnimationSpeed(0.002f);

	jumpAnim = new Animation();
	if (!jumpAnim->Init("data/anims/jump.fbx", 52, false))
		return false;
	jumpAnim->SetAnimationSpeed(0.001f);

	runAnim = new Animation();
	if (!runAnim->Init("data/anims/run.fbx", 52, true))
		return false;

	male->SetAnimation(idleAnim);

	AnimationPack animPack;
	animPack.idleAnimation = idleAnim;
	animPack.walkAnimation = walkAnim;
	animPack.fallAnimation = fallAnim;
	animPack.jumpAnimation = jumpAnim;
	animPack.runAnimation = runAnim;

	player = new Player();
	player->Init(male, physics, animPack);
	player->SetPosition(0.0f, 5.0f, 0.0f);

	GEOMETRY_GENERATE_INFO sphere;
	sphere.type = GEOMETRY_TYPE_SPHERE;
	sphere.radius = 0.25f;
	sphere.slices = 15;
	sphere.stacks = 15;

	debugCameraShadowMap = new WireframeModel();
	if (!debugCameraShadowMap->Init(vulkan, sphere, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)))
	{
		gLogManager->AddMessage("ERROR: Failed to init debug camera model");
		return false;
	}

	return true;
}

void SceneManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(debugCameraShadowMap, vulkan);
	SAFE_UNLOAD(splashScreen, vulkan);

	SAFE_UNLOAD(male, vulkan);
	for (unsigned int i = 0; i < modelList.size(); i++)
		SAFE_UNLOAD(modelList[i], vulkan);

	SAFE_UNLOAD(skydome, vulkan);
	SAFE_UNLOAD(renderDummy, vulkan);

	SAFE_UNLOAD(shadowMaps, vulkan);
	SAFE_UNLOAD(guiManager, vulkan);
	SAFE_UNLOAD(pipelineManager, vulkan);

	for (unsigned int i = 0; i < renderCommandBuffers.size(); i++)
		SAFE_UNLOAD(renderCommandBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(deferredCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(initCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
}

int imageIndex = 5;

void SceneManager::Render(VulkanInterface * vulkan)
{
	// Splash screen
	if (showSplashScreen == true && splashScreenTimer)
			splashScreenTimer->StartTimer();
	
	if (showSplashScreen == true)
	{
		if (splashScreenTimer->TimePassed(1000))
		{
			ChangeGameState(GAME_STATE_LOADING);
			if (LoadGame(vulkan))
				ChangeGameState(GAME_STATE_INGAME);
			else
				THROW_ERROR();

			showSplashScreen = false;
			SAFE_DELETE(splashScreenTimer);
		}
	}

	if (currentGameState == GAME_STATE_INGAME)
	{
		physics->Update();

		glm::vec3 playerPos = player->GetPosition();

		if (playerPos.y < -20.0f)
			player->SetPosition(0.0f, 5.0f, 0.0f);

		if (gInput->WasKeyPressed(KEYBOARD_KEY_E))
		{
			Model * model = new Model();
			model->Init("data/models/box.rcm", vulkan, initCommandBuffer, physics, 20.0f);

			glm::vec3 pos = camera->GetPosition();
			glm::vec3 dir = camera->GetDirection();
			float power = 5.0f;
			model->SetPosition(pos.x, pos.y, pos.z);
			model->SetVelocity(dir.x * power, dir.y * power, dir.z * power);

			modelList.push_back(model);
		}

		if (gInput->WasKeyPressed(KEYBOARD_KEY_O))
		{
			camera->SetCameraState(CAMERA_STATE_ORBIT_PLAYER);
			player->TogglePlayerInput(true);
		}
		if (gInput->WasKeyPressed(KEYBOARD_KEY_P))
		{
			camera->SetCameraState(CAMERA_STATE_FLY);
			player->TogglePlayerInput(false);
		}

		camera->HandleInput();
		light->SetLightDirection(0.0f, -0.5f, 1.0f);

		// Debug deferred shading
		if (gInput->WasKeyPressed(KEYBOARD_KEY_1))
			imageIndex = 1;
		if (gInput->WasKeyPressed(KEYBOARD_KEY_2))
			imageIndex = 2;
		if (gInput->WasKeyPressed(KEYBOARD_KEY_3))
			imageIndex = 3;
		if (gInput->WasKeyPressed(KEYBOARD_KEY_4))
			imageIndex = 4;
		if (gInput->WasKeyPressed(KEYBOARD_KEY_5))
			imageIndex = 5;

		player->Update(vulkan, camera);

		// Shadow pass
		shadowMaps->UpdatePartitions(vulkan, camera, light);

		debugCameraShadowMap->SetPosition(10.0f, 10.0f, 0.0f);
		
		shadowMaps->BeginShadowPass(deferredCommandBuffer);

		for (unsigned int i = 0; i < modelList.size(); i++)
			modelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetShadow(), camera, shadowMaps);

		player->GetModel()->Render(vulkan, deferredCommandBuffer, pipelineManager->GetShadowSkinned(), camera, shadowMaps);

		shadowMaps->EndShadowPass(vulkan->GetVulkanDevice(), deferredCommandBuffer);
		
		// Deferred rendering
		vulkan->BeginSceneDeferred(deferredCommandBuffer);

		for (unsigned int i = 0; i < modelList.size(); i++)
			modelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetDeferred(), camera, shadowMaps);

		player->GetModel()->Render(vulkan, deferredCommandBuffer, pipelineManager->GetSkinned(), camera, NULL);

		vulkan->EndSceneDeferred(deferredCommandBuffer);
	}
	else
	{
		vulkan->BeginSceneDeferred(deferredCommandBuffer);
		vulkan->EndSceneDeferred(deferredCommandBuffer);
	}

	// Forward rendering
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		vulkan->BeginSceneForward(renderCommandBuffers[i], (int)i);
		
		if (currentGameState == GAME_STATE_INGAME)
		{
			skydome->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetSkydome(), camera, (int)i);
			renderDummy->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetDefault(), vulkan->GetOrthoMatrix(),
				light, imageIndex, camera, shadowMaps, (int)i);

			debugCameraShadowMap->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetWireframe(), camera, (int)i);
			shadowMaps->RenderDebug(vulkan, renderCommandBuffers[i], pipelineManager->GetWireframe(), camera, (int)i);
		}
		else if (currentGameState == GAME_STATE_SPLASH_SCREEN)
			splashScreen->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetCanvas(), (int)i);
		else
		{
			gLogManager->AddMessage("ERROR: Unknown game state!");
			THROW_ERROR();
		}

		guiManager->Update(vulkan, renderCommandBuffers[i], pipelineManager->GetCanvas(), (int)i);

		vulkan->EndSceneForward(renderCommandBuffers[i]);
	}
	
	// Present to screen
	vulkan->Present(renderCommandBuffers);
}

bool SceneManager::LoadMapFile(std::string filename, VulkanInterface * vulkan)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		gLogManager->AddMessage("ERROR: Couldn't find map file: " + filename);
		return false;
	}

	while (!file.eof())
	{
		std::string modelPath;
		std::string modelName;
		float posX, posY, posZ;
		float rotX, rotY, rotZ;
		float mass;

		file >> modelName >> posX >> posY >> posZ >> rotX >> rotY >> rotZ >> mass;

		modelName.append(".rcm");

		modelPath = "data/models/" + modelName;

		Model * model = new Model();
		if (!model->Init(modelPath, vulkan, initCommandBuffer, physics, mass))
		{
			gLogManager->AddMessage("ERROR: Failed to init model: " + modelName);
			return false;
		}

		model->SetPosition(posX, posY, posZ);
		model->SetRotation(rotX, rotY, rotZ);

		modelList.push_back(model);
	}

	file.close();
	return true;
}

void SceneManager::ChangeGameState(GAME_STATE newGameState)
{
	lastGameState = currentGameState;
	currentGameState = newGameState;

	if (currentGameState == GAME_STATE_SPLASH_SCREEN)
		gLogManager->AddMessage("GAME STATE: SPLASH SCREEN");
	else if (currentGameState == GAME_STATE_LOADING)
		gLogManager->AddMessage("GAME STATE: LOADING");
	else if (currentGameState == GAME_STATE_INGAME)
		gLogManager->AddMessage("GAME STATE: INGAME");
}
