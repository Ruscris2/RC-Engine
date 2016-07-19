/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Light.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Light.h"

Light::Light()
{
	ambientColor = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);
}

void Light::SetAmbientColor(float r, float g, float b, float a)
{
	ambientColor = glm::vec4(r, g, b, a);
}

void Light::SetDiffuseColor(float r, float g, float b, float a)
{
	diffuseColor = glm::vec4(r, g, b, a);
}

void Light::SetSpecularColor(float r, float g, float b, float a)
{
	specularColor = glm::vec4(r, g, b, a);
}

void Light::SetLightDirection(float x, float y, float z)
{
	lightDirection = glm::vec3(x, y, z);
}

glm::vec4 Light::GetAmbientColor()
{
	return ambientColor;
}

glm::vec4 Light::GetDiffuseColor()
{
	return diffuseColor;
}

glm::vec4 Light::GetSpecularColor()
{
	return specularColor;
}

glm::vec3 Light::GetLightDirection()
{
	return lightDirection;
}
