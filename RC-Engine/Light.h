/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Light.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Light
{
	private:
		glm::vec4 ambientColor;
		glm::vec4 diffuseColor;
		glm::vec4 specularColor;
		glm::vec3 lightDirection;
	public:
		Light();

		void SetAmbientColor(float r, float g, float b, float a);
		void SetDiffuseColor(float r, float g, float b, float a);
		void SetSpecularColor(float r, float g, float b, float a);
		void SetLightDirection(float x, float y, float z);
		glm::vec4 GetAmbientColor();
		glm::vec4 GetDiffuseColor();
		glm::vec4 GetSpecularColor();
		glm::vec3 GetLightDirection();
};