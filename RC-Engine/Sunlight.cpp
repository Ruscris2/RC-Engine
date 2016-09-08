/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Sunlight.cpp                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Sunlight.h"

Sunlight::Sunlight()
{
	ambientColor = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	shadowStrength = 1.0f;
}

void Sunlight::SetAmbientColor(float r, float g, float b, float a)
{
	ambientColor = glm::vec4(r, g, b, a);
}

void Sunlight::SetDiffuseColor(float r, float g, float b, float a)
{
	diffuseColor = glm::vec4(r, g, b, a);
}

void Sunlight::SetSpecularColor(float r, float g, float b, float a)
{
	specularColor = glm::vec4(r, g, b, a);
}

void Sunlight::SetLightDirection(float x, float y, float z)
{
	lightDirection = glm::vec3(x, y, z);
}

void Sunlight::SetShadowStrength(float strength)
{
	shadowStrength = strength;
}

glm::vec4 Sunlight::GetAmbientColor()
{
	return ambientColor;
}

glm::vec4 Sunlight::GetDiffuseColor()
{
	return diffuseColor;
}

glm::vec4 Sunlight::GetSpecularColor()
{
	return specularColor;
}

glm::vec3 Sunlight::GetLightDirection()
{
	return lightDirection;
}

float Sunlight::GetShadowStrength()
{
	return shadowStrength;
}
