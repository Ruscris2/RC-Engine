/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: FrustumCuller.h                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "Camera.h"
#include "Model.h"

class FrustumCuller
{
	private:
		glm::vec4 planes[6];
	public:
		FrustumCuller();

		void BuildFrustum(Camera * camera);
		bool IsInsideFrustum(Model * model);
};