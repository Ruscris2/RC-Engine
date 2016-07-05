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

struct LightBuffer
{
	glm::vec4 ambientColor;
	glm::vec4 diffuseColor;
	glm::vec3 lightDirection;
	float padding;
};

class Light
{
	private:
		LightBuffer lightBuffer;
	public:
		Light();

		void SetAmbientColor(float r, float g, float b, float a);
		void SetDiffuseColor(float r, float g, float b, float a);
		void SetLightDirection(float x, float y, float z);
		size_t GetLightBufferSize();
		LightBuffer * GetLightBuffer();
};