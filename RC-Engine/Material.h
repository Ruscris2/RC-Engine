/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Material.h                                           |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "Texture.h"

class Material
{
	private:
		Texture * diffuseTexture;
		Texture * specularTexture;
		Texture * normalTexture;
		float specularShininess;
		float specularStrength;
	public:
		Material();
		~Material();

		void SetDiffuseTexture(Texture * texture);
		void SetSpecularTexture(Texture * texture);
		void SetNormalTexture(Texture * texture);
		void SetSpecularStrength(float value);
		void SetSpecularShininess(float value);
		Texture * GetDiffuseTexture();
		Texture * GetSpecularTexture();
		Texture * GetNormalTexture();
		float GetSpecularShininess();
		float GetSpecularStrength();
		bool HasSpecularMap();
		bool HasNormalMap();
};