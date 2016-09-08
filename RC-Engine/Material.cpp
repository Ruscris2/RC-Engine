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
	materialTexture = nullptr;
	normalTexture = nullptr;
	metallicOffset = 0.0f;
	roughnessOffset = 0.0f;
}

Material::~Material()
{
	normalTexture = nullptr;
	materialTexture = nullptr;
	diffuseTexture = nullptr;
}

void Material::SetDiffuseTexture(Texture * texture)
{
	diffuseTexture = texture;
}

void Material::SetMaterialTexture(Texture * texture)
{
	materialTexture = texture;
}

void Material::SetNormalTexture(Texture * texture)
{
	normalTexture = texture;
}

void Material::SetMetallicOffset(float value)
{
	metallicOffset = value;
}

void Material::SetRoughnessOffset(float value)
{
	roughnessOffset = value;
}

Texture * Material::GetDiffuseTexture()
{
	return diffuseTexture;
}

Texture * Material::GetMaterialTexture()
{
	return materialTexture;
}

Texture * Material::GetNormalTexture()
{
	return normalTexture;
}

bool Material::HasNormalMap()
{
	if (normalTexture != nullptr)
		return true;

	return false;
}

float Material::GetMetallicOffset()
{
	return metallicOffset;
}

float Material::GetRoughnessOffset()
{
	return roughnessOffset;
}
