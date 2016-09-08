/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#pragma once

#include "Camera.h"
#include "VulkanInterface.h"
#include "PipelineManager.h"
#include "VulkanCommandBuffer.h"
#include "Model.h"
#include "SkinnedModel.h"
#include "WireframeModel.h"
#include "Sunlight.h"
#include "RenderDummy.h"
#include "Animation.h"
#include "Physics.h"
#include "Player.h"
#include "Skydome.h"
#include "Canvas.h"
#include "GameplayTimer.h"
#include "GUIManager.h"
#include "ShadowMaps.h"
#include "FrustumCuller.h"
#include "TimeCycle.h"
#include "LightManager.h"
#include "Cubemap.h"

enum GAME_STATE
{
	GAME_STATE_UNINITIALIZED,
	GAME_STATE_SPLASH_SCREEN,
	GAME_STATE_LOADING,
	GAME_STATE_INGAME
};

class SceneManager
{
	private:
		GAME_STATE currentGameState, lastGameState;

		Physics * physics;
		Camera * camera;
		TimeCycle * timeCycle;
		Sunlight * sunlight;
		PipelineManager * pipelineManager;
		LightManager * lightManager;
		GUIManager * guiManager;
		ShadowMaps * shadowMaps;
		FrustumCuller * frustumCuller;

		VulkanCommandBuffer * initCommandBuffer;
		VulkanCommandBuffer * deferredCommandBuffer;
		std::vector<VulkanCommandBuffer*> renderCommandBuffers;

		RenderDummy * renderDummy;
		Skydome * skydome;

		Animation * idleAnim;
		Animation * walkAnim;
		Animation * fallAnim;
		Animation * jumpAnim;
		Animation * runAnim;

		std::vector<Model*> modelList;
		SkinnedModel * male;
		Player * player;

		GUIElement * splashScreen;
		GameplayTimer * splashScreenTimer;
		bool showSplashScreen;

		Cubemap * testCubemap;
	private:
		bool LoadMapFile(std::string filename, VulkanInterface * vulkan);
		bool LoadGame(VulkanInterface * vulkan);
		void ChangeGameState(GAME_STATE newGameState);
	public:
		SceneManager();
		~SceneManager();

		bool Init(VulkanInterface * vulkan);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan);
};