/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Sunlight.h                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Sunlight
{
	private:
		glm::vec3 lightDirection;
		float lightStrength;
	public:
		Sunlight();

		void SetLightDirection(float x, float y, float z);
		void SetLightStrength(float strength);
		glm::vec3 GetLightDirection();
		float GetLightStrength();
};