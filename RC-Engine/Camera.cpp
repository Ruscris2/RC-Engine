/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Camera.cpp                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Camera.h"
#include "Input.h"
#include "Timer.h"

extern Input * gInput;
extern Timer * gTimer;

void Camera::Init()
{
	position = glm::vec3(0, 0, -5.0f);
	lookAt = glm::vec3(0, 0, 0);
	up = glm::vec3(0, -1, 0);
	direction = glm::vec3(0, 0, 1);
	pitch = roll = 0.0f;
	yaw = 90.0f;
	orbitPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	orbitPointOrientation = glm::vec3(0.0f, 0.0f, 0.0f);
	orbitRadius = 2.0f;
	currentState = CAMERA_STATE_FLY;
	Update();
}

void Camera::HandleInput()
{
	float speed = 0.002f;
	float sensitivity = 0.2f;

	bool update = false;

	if (currentState == CAMERA_STATE_FLY)
	{
		if (gInput->IsKeyPressed(KEYBOARD_KEY_SHIFT))
			speed *= 5.0f;

		if (gInput->GetCursorRelativeX() != 0)
		{
			update = true;
			yaw -= gInput->GetCursorRelativeX() * sensitivity;
		}
		if (gInput->GetCursorRelativeY() != 0)
		{
			update = true;
			pitch -= gInput->GetCursorRelativeY() * sensitivity;
		}

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction = glm::normalize(direction);

		if (gInput->IsKeyPressed(KEYBOARD_KEY_W))
		{
			update = true;
			position = position + (direction * gTimer->GetDelta() * speed);
		}
		if (gInput->IsKeyPressed(KEYBOARD_KEY_S))
		{
			update = true;
			position = position - (direction * gTimer->GetDelta() * speed);
		}
		if (gInput->IsKeyPressed(KEYBOARD_KEY_A))
		{
			update = true;
			position = position + (glm::cross(up, direction) * gTimer->GetDelta() * speed);
		}
		if (gInput->IsKeyPressed(KEYBOARD_KEY_D))
		{
			update = true;
			position = position - (glm::cross(up, direction) * gTimer->GetDelta() * speed);
		}

		lookAt = position + direction;
	}
	else if (currentState == CAMERA_STATE_ORBIT_PLAYER)
	{
		if (gInput->GetCursorRelativeX() != 0)
		{
			update = true;
			yaw -= gInput->GetCursorRelativeX() * sensitivity;
		}
		if (gInput->GetCursorRelativeY() != 0)
		{
			update = true;
			pitch += gInput->GetCursorRelativeY() * sensitivity;
		}

		// Clamp rotation
		if (yaw > 360.0f) yaw = 0.0f;
		if (yaw < 0.0f) yaw = 360.0f;
		if (pitch > 90.0f) pitch = 89.9f;
		if (pitch < -10.0f) pitch = -9.9f;
		

		// Calculate camera position
		float horizontalDistance = orbitRadius * glm::cos(glm::radians(pitch));
		float verticalDistance = orbitRadius * glm::sin(glm::radians(pitch));
		float theta = orbitPointOrientation.y + yaw;

		position.x = orbitPoint.x - (horizontalDistance * glm::sin(glm::radians(theta)));
		position.y = orbitPoint.y + verticalDistance;
		position.z = orbitPoint.z + (horizontalDistance * glm::cos(glm::radians(theta)));

		lookAt = orbitPoint;

		// Update direction
		direction = -glm::normalize(position + lookAt);
	}

	if (update == true)
		Update();
}

void Camera::SetPosition(float x, float y, float z)
{
	position = glm::vec3(x, y, z);
	Update();
}

void Camera::SetDirection(float x, float y, float z)
{
	direction = glm::vec3(x, y, z);
	lookAt = position + direction;
	Update();
}

void Camera::SetOrbitParameters(glm::vec3 orbitPoint, glm::vec3 orbitPointOrientation, float radius)
{
	this->orbitPoint = orbitPoint;
	this->orbitPointOrientation = orbitPointOrientation;
	orbitRadius = radius;
	Update();
}

void Camera::SetCameraState(CAMERA_STATE state)
{
	pitch = yaw = roll = 0.0f;
	if (state == CAMERA_STATE_FLY)
		yaw = 90.0f;

	currentState = state;
	Update();
}

glm::mat4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

glm::vec3 Camera::GetPosition()
{
	return position;
}

glm::vec3 Camera::GetDirection()
{
	return direction;
}

CAMERA_STATE Camera::GetCameraState()
{
	return currentState;
}

void Camera::Update()
{
	viewMatrix = glm::lookAt(position, lookAt, up);
}