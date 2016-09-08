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
#include "TextureManager.h"

TextureManager * gTextureManager;

extern LogManager * gLogManager;
extern Input * gInput;
extern Timer * gTimer;

SceneManager::SceneManager()
{
	lastGameState = currentGameState = GAME_STATE_UNINITIALIZED;

	physics = NULL;
	camera = NULL;
	sunlight = NULL;
	pipelineManager = NULL;

	initCommandBuffer = NULL;
	deferredCommandBuffer = NULL;

	renderDummy = NULL;
	skydome = NULL;
	shadowMaps = NULL;
	frustumCuller = NULL;

	idleAnim = NULL;
	walkAnim = NULL;
	fallAnim = NULL;
	jumpAnim = NULL;
	runAnim = NULL;

	player = NULL;

	splashScreen = NULL;
	showSplashScreen = true;
}

SceneManager::~SceneManager()
{
	SAFE_DELETE(player);

	SAFE_DELETE(runAnim);
	SAFE_DELETE(jumpAnim);
	SAFE_DELETE(fallAnim);
	SAFE_DELETE(walkAnim);
	SAFE_DELETE(idleAnim);

	SAFE_DELETE(sunlight);
	SAFE_DELETE(camera);
	SAFE_DELETE(frustumCuller);
	SAFE_DELETE(timeCycle);
	SAFE_DELETE(physics);
	SAFE_DELETE(gTextureManager);
}

bool SceneManager::Init(VulkanInterface * vulkan)
{
	// Init resource managers
	gTextureManager = new TextureManager();

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

	// Camera setup
	camera = new Camera();
	camera->Init();
	camera->SetPosition(0.0f, 5.0f, -10.0f);
	camera->SetDirection(0.0f, 0.0f, 1.0f);
	camera->SetCameraState(CAMERA_STATE_ORBIT_PLAYER);

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
	if (!shadowMaps->Init(vulkan, initCommandBuffer, camera))
	{
		gLogManager->AddMessage("ERROR: Failed to init shadow maps!");
		return false;
	}

	// Init light manager
	lightManager = new LightManager();
	if (!lightManager->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init light manager!");
		return false;
	}

	// Physics init
	physics = new Physics();
	physics->Init();

	// Init frustum culler
	frustumCuller = new FrustumCuller();

	// Light setup
	sunlight = new Sunlight();

	// Init test cubemap
	testCubemap = new Cubemap();
	if (!testCubemap->Init(vulkan->GetVulkanDevice(), initCommandBuffer, "data/cubemaps/testcubemap"))
	{
		gLogManager->AddMessage("ERROR: Failed to init cubemap!");
		return false;
	}

	if (!pipelineManager->InitGamePipelines(vulkan, shadowMaps))
	{
		gLogManager->AddMessage("ERROR: Failed to init game pipelines!");
		return false;
	}

	// Init render dummy
	renderDummy = new RenderDummy();
	if (!renderDummy->Init(vulkan, pipelineManager->GetDefault(), vulkan->GetPositionAttachment()->GetImageView(), vulkan->GetNormalAttachment()->GetImageView(),
		vulkan->GetAlbedoAttachment()->GetImageView(), vulkan->GetMaterialAttachment()->GetImageView(), vulkan->GetDepthAttachment()->GetImageView(),
		shadowMaps, lightManager, testCubemap->GetImageView()))
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
	skydome->SetGroundColor(0.2f, 0.2f, 0.2f, 1.0f);
	
	// Init timecycle
	timeCycle = new TimeCycle();
	if (!timeCycle->Init(skydome, sunlight))
	{
		gLogManager->AddMessage("ERROR: Failed to init timecycle!");
		return false;
	}
	timeCycle->SetTime(9, 0);
	timeCycle->SetWeather("sunny");

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

	return true;
}

void SceneManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(testCubemap, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(splashScreen, vulkan);

	SAFE_UNLOAD(male, vulkan);
	for (unsigned int i = 0; i < modelList.size(); i++)
		SAFE_UNLOAD(modelList[i], vulkan);

	SAFE_UNLOAD(skydome, vulkan);
	SAFE_UNLOAD(renderDummy, vulkan);

	SAFE_UNLOAD(lightManager, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(shadowMaps, vulkan);
	SAFE_UNLOAD(guiManager, vulkan);
	SAFE_UNLOAD(pipelineManager, vulkan);

	for (unsigned int i = 0; i < renderCommandBuffers.size(); i++)
		SAFE_UNLOAD(renderCommandBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(deferredCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(initCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
}

int imageIndex = 5;

float metallic = 0.0f;
float roughness = 0.0f;

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
		frustumCuller->BuildFrustum(camera);
		timeCycle->Update();

		glm::vec3 playerPos = player->GetPosition();

		if (playerPos.y < -50.0f)
			player->SetPosition(0.0f, 5.0f, 0.0f);

		if (gInput->IsKeyPressed(KEYBOARD_KEY_T))
			metallic -= 0.005f;
		if (gInput->IsKeyPressed(KEYBOARD_KEY_Y))
			metallic += 0.005f;
		if (gInput->IsKeyPressed(KEYBOARD_KEY_U))
			roughness -= 0.005f;
		if (gInput->IsKeyPressed(KEYBOARD_KEY_I))
			roughness += 0.005f;
		if (gInput->WasKeyPressed(KEYBOARD_KEY_G))
		{
			gLogManager->AddMessage("METALLIC:");
			gLogManager->PrintValue(metallic);
			gLogManager->AddMessage("ROUGHNESS:");
			gLogManager->PrintValue(roughness);
		}

		if (metallic < 0.0f) metallic = 0.0f;
		if (metallic > 1.0f) metallic = 1.0f;
		if (roughness < 0.0f) roughness = 0.0f;
		if (roughness > 1.0f) roughness = 1.0f;

		sunlight->SetSpecularColor(metallic, roughness, 0.0f, 0.0f);

		if (gInput->WasKeyPressed(KEYBOARD_KEY_E))
		{
			Model * model = new Model();
			model->Init("data/models/box.rcm", vulkan, initCommandBuffer, physics, 100.0f);

			glm::vec3 pos = camera->GetPosition();
			glm::vec3 dir = camera->GetDirection();
			float power = 5.0f;
			model->SetPosition(pos.x, pos.y, pos.z);
			model->SetVelocity(dir.x * power, dir.y * power, dir.z * power);

			modelList.push_back(model);
		}

		if (gInput->WasKeyPressed(KEYBOARD_KEY_R))
		{
			Model * model = new Model();
			model->Init("data/models/teapot.rcm", vulkan, initCommandBuffer, physics, 1.0f);

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
		if (gInput->WasKeyPressed(KEYBOARD_KEY_Q))
		{
			char msg[64];
			sprintf(msg, "OBJ: %zu TXD: %zu", modelList.size(), gTextureManager->GetLoadedTexturesCount());
			gLogManager->AddMessage(msg);
		}
		if (gInput->WasKeyPressed(KEYBOARD_KEY_Z))
		{
			Light * light = new Light();
			light->SetLightRadius(15.0f);
			light->SetLightColor(glm::vec4(20.0f, 0.0f, 0.0f, 1.0f));
			light->SetLightPosition(camera->GetPosition());
			lightManager->AddLightToScene(vulkan->GetVulkanDevice(), light);
			gLogManager->AddMessage("LIGHT ADDED");
		}
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
		if (gInput->WasKeyPressed(KEYBOARD_KEY_5))
			imageIndex = 5;

		player->Update(vulkan, camera);

		// Shadow pass
		shadowMaps->UpdatePartitions(vulkan, camera, sunlight);
		
		shadowMaps->BeginShadowPass(deferredCommandBuffer);

		for (unsigned int i = 0; i < modelList.size(); i++)
			modelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetShadow(), NULL, shadowMaps);

		player->GetModel()->Render(vulkan, deferredCommandBuffer, pipelineManager->GetShadowSkinned(), NULL, shadowMaps);

		shadowMaps->EndShadowPass(vulkan->GetVulkanDevice(), deferredCommandBuffer);
		
		// Deferred rendering
		vulkan->BeginSceneDeferred(deferredCommandBuffer);

		for (unsigned int i = 0; i < modelList.size(); i++)
			if (frustumCuller->IsInsideFrustum(modelList[i]))
				modelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetDeferred(), camera, NULL);

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
			renderDummy->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetDefault(), camera->GetOrthoMatrix(),
				sunlight, imageIndex, camera, shadowMaps, (int)i);
		}
		else if (currentGameState == GAME_STATE_SPLASH_SCREEN)
			splashScreen->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetCanvas(), camera, (int)i);
		else
		{
			gLogManager->AddMessage("ERROR: Unknown game state!");
			THROW_ERROR();
		}

		guiManager->Update(vulkan, renderCommandBuffers[i], pipelineManager->GetCanvas(), camera, (int)i);

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
