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
	baseSpeed = 0.002f;
	sensitivity = 0.2f;
	HandleInput();
}

void Camera::HandleInput()
{
	if (currentState == CAMERA_STATE_FLY)
	{
		float sprintMultiplier = 1.0f;

		if (gInput->IsKeyPressed(KEYBOARD_KEY_SHIFT))
			sprintMultiplier = 5.0f;

		yaw -= gInput->GetCursorRelativeX() * sensitivity;
		pitch -= gInput->GetCursorRelativeY() * sensitivity;

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction = glm::normalize(direction);

		if (gInput->IsKeyPressed(KEYBOARD_KEY_W))
			position += direction * gTimer->GetDelta() * baseSpeed * sprintMultiplier;
		if (gInput->IsKeyPressed(KEYBOARD_KEY_S))
			position -= direction * gTimer->GetDelta() * baseSpeed * sprintMultiplier;
		if (gInput->IsKeyPressed(KEYBOARD_KEY_A))
			position += glm::cross(up, direction) * gTimer->GetDelta() * baseSpeed * sprintMultiplier;
		if (gInput->IsKeyPressed(KEYBOARD_KEY_D))
			position -= glm::cross(up, direction) * gTimer->GetDelta() * baseSpeed * sprintMultiplier;

		lookAt = position + direction;
	}
	else if (currentState == CAMERA_STATE_ORBIT_PLAYER)
	{
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

	viewMatrix = glm::lookAt(position, lookAt, up);
}

void Camera::SetPosition(float x, float y, float z)
{
	position = glm::vec3(x, y, z);
}

void Camera::SetDirection(float x, float y, float z)
{
	direction = glm::vec3(x, y, z);
	lookAt = position + direction;
}

void Camera::SetPitch(float pitch)
{
	this->pitch = pitch;
}

void Camera::SetYaw(float yaw)
{
	this->yaw = yaw;
}

void Camera::SetOrbitParameters(glm::vec3 orbitPoint, glm::vec3 orbitPointOrientation, float radius)
{
	this->orbitPoint = orbitPoint;
	this->orbitPointOrientation = orbitPointOrientation;
	orbitRadius = radius;
}

void Camera::SetCameraState(CAMERA_STATE state)
{
	pitch = yaw = roll = 0.0f;
	if (state == CAMERA_STATE_FLY)
		yaw = 90.0f;

	currentState = state;
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

float Camera::GetSensitivity()
{
	return sensitivity;
}
