/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Camera.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Camera
{
	private:
		glm::mat4 viewMatrix;
		glm::vec3 position;
		glm::vec3 lookAt;
		glm::vec3 up;
		glm::vec3 direction;
		float pitch, yaw, roll;
	private:
		void Update();
	public:
		void Init();
		void HandleInput();
		glm::mat4 GetViewMatrix();
		glm::vec3 GetPosition();
};