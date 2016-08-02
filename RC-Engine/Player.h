/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Player.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include "SkinnedModel.h"
#include "Physics.h"

#pragma once

class Player
{
	private:
		SkinnedModel * playerModel;

		Physics * physics;
		btKinematicCharacterController * character;
		btPairCachingGhostObject * ghostObject;
		btConvexShape * capsule;
		
		glm::mat4 worldMatrix;

		bool inputEnabled;
	public:
		Player();
		~Player();

		void Init(SkinnedModel * model, Physics * physics);
		void SetPosition(float x, float y, float z);
		void Update(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera);
		void TogglePlayerInput(bool toggle);
		glm::vec3 GetPosition();
};