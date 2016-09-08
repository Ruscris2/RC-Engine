/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Light.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Light.h"

Light::Light()
{
	lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	radius = 1.0f;
}

void Light::SetLightColor(glm::vec4 color)
{
	lightColor = color;
}

void Light::SetLightPosition(glm::vec3 pos)
{
	lightPosition = pos;
}

void Light::SetLightRadius(float radius)
{
	this->radius = radius;
}

glm::vec4 Light::GetLightColor()
{
	return lightColor;
}

glm::vec3 Light::GetLightPosition()
{
	return lightPosition;
}

float Light::GetLightRadius()
{
	return radius;
}
