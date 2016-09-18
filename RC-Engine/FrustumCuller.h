/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: FrustumCuller.h                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "glm.hpp"

class FrustumCuller
{
	private:
		glm::vec4 planes[6];
	public:
		FrustumCuller();

		void BuildFrustum(glm::mat4 viewProjMatrix);
		bool IsInsideFrustum(class Model * model);
};