/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Player.cpp                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Player.h"
#include "Input.h"
#include "Timer.h"
#include "StdInc.h"

extern Input * gInput;
extern Timer * gTimer;

Player::Player()
{
	playerModel = NULL;
	inputEnabled = true;
}

Player::~Player()
{
	playerModel = NULL;

	physics->GetDynamicsWorld()->removeCollisionObject(ghostObject);
	physics->GetDynamicsWorld()->removeAction(character);

	SAFE_DELETE(character);
	SAFE_DELETE(ghostObject);
	SAFE_DELETE(capsule);
}

void Player::Init(SkinnedModel * model, Physics * physics)
{
	this->physics = physics;

	playerModel = model;

	btTransform transform;
	transform.setIdentity();

	ghostObject = new btPairCachingGhostObject();
	ghostObject->setWorldTransform(transform);

	capsule = new btCapsuleShape(0.3f, 1.2f);

	ghostObject->setCollisionShape(capsule);
	ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	btScalar stepHeight = btScalar(0.1f);
	character = new btKinematicCharacterController(ghostObject, capsule, stepHeight);

	physics->GetDynamicsWorld()->addCollisionObject(ghostObject, btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	physics->GetDynamicsWorld()->addAction(character);
}

void Player::SetPosition(float x, float y, float z)
{
	character->warp(btVector3(x, y, z));
}

void Player::Update(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera)
{
	btTransform transform;
	transform = ghostObject->getWorldTransform();
	
	btVector3 forwardDir = transform.getBasis()[2];

	btVector3 walkDirection = btVector3(0.0f, 0.0f, 0.0f);
	btScalar walkSpeed = 0.05f * gTimer->GetDelta();

	forwardDir.normalize();

	if (inputEnabled)
	{
		if (gInput->IsKeyPressed(KEYBOARD_KEY_A))
		{
			btMatrix3x3 orientation = ghostObject->getWorldTransform().getBasis();
			orientation *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), 0.005f));
			ghostObject->getWorldTransform().setBasis(orientation);
		}
		if (gInput->IsKeyPressed(KEYBOARD_KEY_D))
		{
			btMatrix3x3 orientation = ghostObject->getWorldTransform().getBasis();
			orientation *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), -0.005f));
			ghostObject->getWorldTransform().setBasis(orientation);
		}
		if (gInput->IsKeyPressed(KEYBOARD_KEY_W))
			walkDirection += forwardDir;

		character->setWalkDirection(walkDirection * walkSpeed);
		if (gInput->IsKeyPressed(KEYBOARD_KEY_SPACE))
			character->jump();
	}

	// Pass player info to camera
	if (camera->GetCameraState() == CAMERA_STATE_ORBIT_PLAYER)
	{
		camera->SetOrbitParameters(GetPosition(), glm::vec3(0.0f, 0.0f, 0.0f), 5.0f);
	}

	ghostObject->getWorldTransform().getOpenGLMatrix((btScalar*)&worldMatrix);

	// Adjust position offset to model and convert rotation from right-handed to left-handed
	worldMatrix = glm::translate(worldMatrix, glm::vec3(0.0f, -0.9f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat3 tmp = glm::mat3x3(worldMatrix);
	tmp = glm::inverse(tmp);
	worldMatrix[0][0] = tmp[0][0];
	worldMatrix[0][1] = tmp[0][1];
	worldMatrix[0][2] = tmp[0][2];
	worldMatrix[1][0] = tmp[1][0];
	worldMatrix[1][1] = tmp[1][1];
	worldMatrix[1][2] = tmp[1][2];
	worldMatrix[2][0] = tmp[2][0];
	worldMatrix[2][1] = tmp[2][1];
	worldMatrix[2][2] = tmp[2][2];

	// Render
	playerModel->Render(vulkan, commandBuffer, vulkanPipeline, camera, worldMatrix);
}

void Player::TogglePlayerInput(bool toggle)
{
	inputEnabled = toggle;
}

glm::vec3 Player::GetPosition()
{
	btTransform transform;
	transform = ghostObject->getWorldTransform();

	glm::vec3 position;
	position.x = transform.getOrigin().getX();
	position.y = transform.getOrigin().getY();
	position.z = transform.getOrigin().getZ();

	return position;
}
