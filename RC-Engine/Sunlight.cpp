/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Sunlight.cpp                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Sunlight.h"

Sunlight::Sunlight()
{
	lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	lightStrength = 1.0f;
}

void Sunlight::SetLightDirection(float x, float y, float z)
{
	lightDirection = glm::vec3(x, y, z);
}

void Sunlight::SetLightStrength(float strength)
{
	lightStrength = strength;
}

glm::vec3 Sunlight::GetLightDirection()
{
	return lightDirection;
}

float Sunlight::GetLightStrength()
{
	return lightStrength;
}
