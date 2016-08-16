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
	baseOrientation = 0.0f;
	strafeOrientation = 0.0f;
	playerOrientation = 0.0f;
	cameraYaw = 0.0f;
	cameraPitch = 0.0f;
	playerMoving = false;
	isJumping = false;
	runKeyPressed = false;
	velocityStopped = false;
}

Player::~Player()
{
	playerModel = NULL;

	physics->GetDynamicsWorld()->removeRigidBody(playerBody);
	delete playerBody->getMotionState();
	SAFE_DELETE(playerBody);
	SAFE_DELETE(capsuleShape);
}

void Player::Init(SkinnedModel * model, Physics * physics, AnimationPack animPack)
{
	playerModel = model;

	this->physics = physics;
	this->animPack = animPack;

	// Player physics controller
	capsuleShape = new btCapsuleShape(0.3f, 1.2f);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(0.0f, 5.0f, 0.0f));

	btDefaultMotionState * motionState = new btDefaultMotionState(transform);

	btScalar mass = 60.0f;
	btVector3 inertia(0.0f, 0.0f, 0.0f);
	capsuleShape->calculateLocalInertia(mass, inertia);

	btRigidBody::btRigidBodyConstructionInfo bodyCI(mass, motionState, capsuleShape, inertia);
	playerBody = new btRigidBody(bodyCI);

	playerBody->setSleepingThresholds(0.0f, 0.0f);
	playerBody->setAngularFactor(0.0f);
	playerBody->forceActivationState(DISABLE_DEACTIVATION);
	playerBody->setCcdSweptSphereRadius(0.5f);
	playerBody->setCcdMotionThreshold(0.00001f);
	physics->GetDynamicsWorld()->addRigidBody(playerBody);

	walkSpeed = 2.5f;
	runSpeed = 7.5f;
	baseDirection = btVector3(0.0f, 0.0f, 1.0f);

	animPack.walkAnimation->SetAnimationSpeed(0.001f + (0.0005f * walkSpeed));
	animPack.runAnimation->SetAnimationSpeed(0.001f + (0.0002f * runSpeed));
}

void Player::SetPosition(float x, float y, float z)
{
	physics->GetDynamicsWorld()->removeRigidBody(playerBody);

	btTransform transform;
	playerBody->getMotionState()->getWorldTransform(transform);
	transform.setOrigin(btVector3(x, y, z));
	playerBody->getMotionState()->setWorldTransform(transform);
	playerBody->setCenterOfMassTransform(transform);

	physics->GetDynamicsWorld()->addRigidBody(playerBody);

	cameraYaw = 0.0f;
	cameraPitch = 0.0f;
	strafeOrientation = 0.0f;
	playerOrientation = 0.0f;
}

void Player::Update(VulkanInterface * vulkan, Camera * camera)
{
	TrackJumpState();

	if (inputEnabled)
	{
		cameraYaw -= gInput->GetCursorRelativeX() * camera->GetSensitivity();
		cameraPitch += gInput->GetCursorRelativeY() * camera->GetSensitivity();
		
		// Clamp camera rotation
		if (cameraYaw < 0.0f) cameraYaw = 359.9f;
		if (cameraYaw > 360.0f) cameraYaw = 0.1f;
		if (cameraPitch > 90.0f) cameraPitch = 89.9f;
		if (cameraPitch < -10.0f) cameraPitch = -9.9f;

		forwardKeyPressed = (gInput->IsKeyPressed(KEYBOARD_KEY_W) ? true : false);
		leftKeyPressed = (gInput->IsKeyPressed(KEYBOARD_KEY_A) ? true : false);
		rightKeyPressed = (gInput->IsKeyPressed(KEYBOARD_KEY_D) ? true : false);
		backwardsKeyPressed = (gInput->IsKeyPressed(KEYBOARD_KEY_S) ? true : false);
		runKeyPressed = (gInput->IsKeyPressed(KEYBOARD_KEY_SHIFT) ? true : false);

		// Are there any opposing keys pressed at the same time?
		if (forwardKeyPressed && backwardsKeyPressed)
		{
			forwardKeyPressed = false;
			backwardsKeyPressed = false;
		}
		if (leftKeyPressed && rightKeyPressed)
		{
			leftKeyPressed = false;
			rightKeyPressed = false;
		}

		// If any key is set to true, than the player is moving
		if (forwardKeyPressed || leftKeyPressed || rightKeyPressed || backwardsKeyPressed)
			playerMoving = true;
		else
			playerMoving = false;
	}

	btTransform transform;
	playerBody->getMotionState()->getWorldTransform(transform);

	walkDirection = btVector3(0.0f, 0.0f, 0.0f);

	// Update orientation based on key pressed
	if (forwardKeyPressed && rightKeyPressed)
		strafeOrientation = 45.0f;
	else if (forwardKeyPressed && leftKeyPressed)
		strafeOrientation = 315.0f;
	else if (backwardsKeyPressed && rightKeyPressed)
		strafeOrientation = 135.0f;
	else if (backwardsKeyPressed && leftKeyPressed)
		strafeOrientation = 225.0f;
	else if (backwardsKeyPressed)
		strafeOrientation = 180.0f;
	else if (rightKeyPressed)
		strafeOrientation = 90.0f;
	else if (leftKeyPressed)
		strafeOrientation = 270.0f;
	else if (forwardKeyPressed)
		strafeOrientation = 0.0f;

	if (!playerMoving && playerOrientation != 0.0f)
	{
		playerOrientation += gInput->GetCursorRelativeX() * camera->GetSensitivity();
		if (playerOrientation < 0.0f || playerOrientation > 360.0f)
			playerOrientation = 0.0f;

		// Negate last camera yaw change
		cameraYaw += gInput->GetCursorRelativeX() * camera->GetSensitivity();
	}
	
	baseOrientation = -cameraYaw;

	if (playerMoving)
		playerOrientation = InterpolateRotation(playerOrientation, strafeOrientation, 0.5f);

	transform.setRotation(btQuaternion(btVector3(0.0f, 1.0f, 0.0f), glm::radians(baseOrientation + playerOrientation)));
	playerBody->getMotionState()->setWorldTransform(transform);

	btVector3 velocity = playerBody->getLinearVelocity();

	if ((forwardKeyPressed || leftKeyPressed || rightKeyPressed || backwardsKeyPressed) && inputEnabled)
	{
		walkDirection += baseDirection * (runKeyPressed ? runSpeed : walkSpeed) * gTimer->GetDelta();
		walkDirection.setY(velocity.y());

		playerBody->setLinearVelocity(RotateVec3Quaternion(walkDirection, transform.getRotation()));
		velocityStopped = false;
	}
	else if (velocityStopped == false)
	{
		playerBody->setLinearVelocity(btVector3(0.0f, velocity.y(), 0.0f));
		velocityStopped = true;
	}

	if (inputEnabled)
	{
		if (gInput->IsKeyPressed(KEYBOARD_KEY_SPACE))
			Jump();
	}

	// Pass player info to camera
	if (camera->GetCameraState() == CAMERA_STATE_ORBIT_PLAYER)
	{
		camera->SetOrbitParameters(GetPosition(), glm::vec3(0.0f, 180.0f, 0.0f), 5.0f);
		camera->SetYaw(cameraYaw);
		camera->SetPitch(cameraPitch);
	}

	playerBody->getMotionState()->getWorldTransform(transform);
	transform.getOpenGLMatrix((btScalar*)&worldMatrix);

	// Adjust position offset to model
	worldMatrix = glm::translate(worldMatrix, glm::vec3(0.0f, -0.9f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Apply anim
	if (IsFalling(2.5f))
		playerModel->SetAnimation(animPack.fallAnimation);
	else if (playerMoving && runKeyPressed && !isJumping)
		playerModel->SetAnimation(animPack.runAnimation);
	else if (playerMoving && !isJumping)
		playerModel->SetAnimation(animPack.walkAnimation);
	else if(!isJumping)
		playerModel->SetAnimation(animPack.idleAnimation);

	playerModel->SetWorldMatrix(worldMatrix);
	playerModel->UpdateAnimation(vulkan);
}

void Player::TogglePlayerInput(bool toggle)
{
	inputEnabled = toggle;
}

void Player::Jump()
{
	if (isJumping == false && !IsFalling(0.2f))
	{
		btVector3 velocity = playerBody->getLinearVelocity();
		playerBody->setLinearVelocity(btVector3(velocity.x(), 5.0f, velocity.z()));
		isJumping = true;
		jumpTracking_FallingInitiated = false;

		animPack.jumpAnimation->ResetAnimation();
		playerModel->SetAnimation(animPack.jumpAnimation);
	}
}

void Player::TrackJumpState()
{
	if (isJumping)
	{
		if (IsFalling(0.2f))
			jumpTracking_FallingInitiated = true;
		else if (!IsFalling(0.2f) && jumpTracking_FallingInitiated == true)
			isJumping = false;
	}
}

glm::vec3 Player::GetPosition()
{
	btTransform transform;
	playerBody->getMotionState()->getWorldTransform(transform);

	glm::vec3 position;
	position.x = transform.getOrigin().getX();
	position.y = transform.getOrigin().getY();
	position.z = transform.getOrigin().getZ();

	return position;
}

bool Player::IsFalling(float errorMargin)
{
	btScalar downVelocity = playerBody->getLinearVelocity().getY();
	if (downVelocity < -errorMargin)
		return true;

	return false;
}

SkinnedModel * Player::GetModel()
{
	return playerModel;
}

btVector3 Player::RotateVec3Quaternion(btVector3 vec, btQuaternion quat)
{
	btVector3 qv(quat.x(), quat.y(), quat.z());
	btVector3 uv = qv.cross(vec);
	btVector3 uuv = qv.cross(uv);
	uv *= (2.0f * quat.w());
	uuv *= 2.0f;

	return vec + uv + uuv;
}

float Player::InterpolateRotation(float currentRotation, float targetRotation, float speed)
{
	if (glm::abs(currentRotation - targetRotation) < speed * gTimer->GetDelta())
		return targetRotation;

	int currentRotationQuarter = GetCircleQuarter(currentRotation);
	int targetRotationQuarter = GetCircleQuarter(targetRotation);
	float sign;

	if (currentRotationQuarter == targetRotationQuarter)
	{
		if (currentRotation < targetRotation)
			sign = 1.0f;
		else
			sign = -1.0f;
	}
	else if (currentRotationQuarter == 4 && targetRotationQuarter == 1)
		sign = 1.0f;
	else if (currentRotationQuarter == 1 && targetRotationQuarter == 4)
		sign = -1.0f;
	else if ((currentRotationQuarter == 2 && targetRotationQuarter == 4) || (currentRotationQuarter == 1 && targetRotationQuarter == 3))
	{
		if (currentRotation - targetRotation > 360.0f - currentRotation + targetRotation)
			sign = -1.0f;
		else
			sign = 1.0f;
	}
	else if ((currentRotationQuarter == 4 && targetRotationQuarter == 2) || (currentRotationQuarter == 3 && targetRotationQuarter == 1))
	{
		if (currentRotation - targetRotation > 360.0f - currentRotation + targetRotation)
			sign = 1.0f;
		else
			sign = -1.0f;
	}
	else if (currentRotationQuarter > targetRotationQuarter)
		sign = -1.0f;
	else
		sign = 1.0f;

	currentRotation += sign * speed * gTimer->GetDelta();

	if (currentRotation > 360.0f)
		currentRotation = 0.1f;
	if (currentRotation < 0.0f)
		currentRotation = 359.9f;

	return currentRotation;
}

int Player::GetCircleQuarter(float rotation)
{
	if (rotation >= 0.0f && rotation < 90.0f)
		return 1;
	else if (rotation >= 90.0f && rotation < 180.0f)
		return 2;
	else if (rotation >= 180.0f && rotation < 270.0f)
		return 3;
	else
		return 4;
}