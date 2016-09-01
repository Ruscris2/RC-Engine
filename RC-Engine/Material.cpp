#include "Material.h"
/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Material.cpp                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

Material::Material()
{
	diffuseTexture = nullptr;
	specularTexture = nullptr;
	normalTexture = nullptr;
	specularShininess = 32.0f;
	specularStrength = 1.0f;
}

Material::~Material()
{
	specularTexture = nullptr;
	diffuseTexture = nullptr;
}

void Material::SetDiffuseTexture(Texture * texture)
{
	diffuseTexture = texture;
}

void Material::SetSpecularTexture(Texture * texture)
{
	specularTexture = texture;
}

void Material::SetNormalTexture(Texture * texture)
{
	normalTexture = texture;
}

void Material::SetSpecularStrength(float value)
{
	specularStrength = value;
}

void Material::SetSpecularShininess(float value)
{
	specularShininess = value;
}

Texture * Material::GetDiffuseTexture()
{
	return diffuseTexture;
}

Texture * Material::GetSpecularTexture()
{
	return specularTexture;
}

Texture * Material::GetNormalTexture()
{
	return normalTexture;
}

float Material::GetSpecularShininess()
{
	return specularShininess;
}

float Material::GetSpecularStrength()
{
	return specularStrength;
}

bool Material::HasSpecularMap()
{
	if (specularTexture != nullptr)
		return true;

	return false;
}

bool Material::HasNormalMap()
{
	if (normalTexture != nullptr)
		return true;

	return false;
}
