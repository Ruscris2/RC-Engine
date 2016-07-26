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
#include <gtc/quaternion.hpp>

enum CAMERA_STATE
{
	CAMERA_STATE_FLY,
	CAMERA_STATE_ORBIT_PLAYER
};

class Camera
{
	private:
		glm::mat4 viewMatrix;
		glm::vec3 position;
		glm::vec3 lookAt;
		glm::vec3 up;
		glm::vec3 direction;
		glm::vec3 orbitPoint;
		glm::vec3 orbitPointOrientation;
		float pitch, yaw, roll;
		float orbitRadius;

		CAMERA_STATE currentState;
	private:
		void Update();
	public:
		void Init();
		void HandleInput();
		void SetPosition(float x, float y, float z);
		void SetDirection(float x, float y, float z);
		void SetOrbitParameters(glm::vec3 orbitPoint, glm::vec3 orbitPointOrientation, float radius);
		void SetCameraState(CAMERA_STATE state);
		glm::mat4 GetViewMatrix();
		glm::vec3 GetPosition();
		glm::vec3 GetDirection();
		CAMERA_STATE GetCameraState();
};