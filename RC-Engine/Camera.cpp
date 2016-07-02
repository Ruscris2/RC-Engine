/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Camera.cpp                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Camera.h"
#include "Input.h"

extern Input * gInput;

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
	bool update = false;

	if (gInput->IsKeyPressed(MOUSE_LEFTBUTTON))
	{
		if (gInput->GetCursorRelativeX() != 0)
		{
			update = true;
			yaw -= gInput->GetCursorRelativeX() * 0.2f;
		}
		if (gInput->GetCursorRelativeY() != 0)
		{
			update = true;
			pitch -= gInput->GetCursorRelativeY() * 0.2f;
		}
	}

	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction = glm::normalize(direction);

	if (gInput->IsKeyPressed(KEYBOARD_KEY_W))
	{
		update = true;
		position = position + (direction * 0.01f);
	}
	if (gInput->IsKeyPressed(KEYBOARD_KEY_S))
	{
		update = true;
		position = position - (direction * 0.01f);
	}
	if (gInput->IsKeyPressed(KEYBOARD_KEY_A))
	{
		update = true;
		position = position + glm::cross(up, direction) * 0.01f;
	}
	if (gInput->IsKeyPressed(KEYBOARD_KEY_D))
	{
		update = true;
		position = position - glm::cross(up, direction) * 0.01f;
	}

	lookAt = position + direction;
	if (update == true)
		Update();
}

glm::mat4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

void Camera::Update()
{
	viewMatrix = glm::lookAt(position, lookAt, up);
}