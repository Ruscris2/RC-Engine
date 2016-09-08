/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Light.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#pragma once

#include <glm.hpp>

class Light
{
	private:
		glm::vec4 lightColor;
		glm::vec3 lightPosition;
		float radius;
	public:
		Light();

		void SetLightColor(glm::vec4 color);
		void SetLightPosition(glm::vec3 pos);
		void SetLightRadius(float radius);
		glm::vec4 GetLightColor();
		glm::vec3 GetLightPosition();
		float GetLightRadius();
};