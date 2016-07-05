/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Light.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Light.h"

Light::Light()
{
	lightBuffer.ambientColor = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
	lightBuffer.diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightBuffer.lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	lightBuffer.padding = 0.0f;
}

void Light::SetAmbientColor(float r, float g, float b, float a)
{
	lightBuffer.ambientColor = glm::vec4(r, g, b, a);
}

void Light::SetDiffuseColor(float r, float g, float b, float a)
{
	lightBuffer.diffuseColor = glm::vec4(r, g, b, a);
}

void Light::SetLightDirection(float x, float y, float z)
{
	lightBuffer.lightDirection = glm::vec3(x, y, z);
}

size_t Light::GetLightBufferSize()
{
	return sizeof(lightBuffer);
}

LightBuffer * Light::GetLightBuffer()
{
	return &lightBuffer;
}
