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
	Update();
}

void Camera::HandleInput()
{
	float speed = 0.006f;
	float sensitivity = 0.2f;

	bool update = false;

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
	if (update == true)
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

void Camera::Update()
{
	viewMatrix = glm::lookAt(position, lookAt, up);
}