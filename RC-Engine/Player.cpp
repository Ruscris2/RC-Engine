/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Player.cpp                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Player.h"
#include "Input.h"

extern Input * gInput;

Player::Player()
{
	playerModel = NULL;
}

Player::~Player()
{
	playerModel = NULL;
}

void Player::Init(SkinnedModel * model)
{
	playerModel = model;
}

void Player::SetPosition(float x, float y, float z)
{
	playerModel->SetPosition(x, y, z);
}

float Player::SetDirection(float angle)
{
	playerModel->SetRotation(0.0f, angle, 0.0f);
}

void Player::Update(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera)
{
	// Pass player info to camera
	if (camera->GetCameraState() == CAMERA_STATE_ORBIT_PLAYER)
	{
		// Add one meter offset to look position
		glm::vec3 position = playerModel->GetPosition();
		glm::vec3 rotation = playerModel->GetRotation();
		position.y += 1.0f;

		camera->SetOrbitParameters(position, rotation, 5.0f);
	}

	// Render
	playerModel->Render(vulkan, commandBuffer, vulkanPipeline, camera);
}
