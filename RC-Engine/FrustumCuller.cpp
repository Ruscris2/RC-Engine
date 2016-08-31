/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: FrustumCuller.cpp                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "FrustumCuller.h"

FrustumCuller::FrustumCuller()
{
	for(int i = 0; i < 6; i++)
		planes[i] = glm::vec4();
}

void FrustumCuller::BuildFrustum(Camera * camera)
{
	float length;
	glm::mat4 viewProjMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix();

	// Near plane
	planes[0][0] = viewProjMatrix[0][3] + viewProjMatrix[0][2];
	planes[0][1] = viewProjMatrix[1][3] + viewProjMatrix[1][2];
	planes[0][2] = viewProjMatrix[2][3] + viewProjMatrix[2][2];
	planes[0][3] = viewProjMatrix[3][3] + viewProjMatrix[3][2];
	
	length = sqrtf(planes[0][0] * planes[0][0] + planes[0][1] * planes[0][1] + planes[0][2] * planes[0][2]);
	planes[0][0] /= length;
	planes[0][1] /= length;
	planes[0][2] /= length;
	planes[0][3] /= length;

	// Far plane
	planes[1][0] = viewProjMatrix[0][3] - viewProjMatrix[0][2];
	planes[1][1] = viewProjMatrix[1][3] - viewProjMatrix[1][2];
	planes[1][2] = viewProjMatrix[2][3] - viewProjMatrix[2][2];
	planes[1][3] = viewProjMatrix[3][3] - viewProjMatrix[3][2];

	length = sqrtf(planes[1][0] * planes[1][0] + planes[1][1] * planes[1][1] + planes[1][2] * planes[1][2]);
	planes[1][0] /= length;
	planes[1][1] /= length;
	planes[1][2] /= length;
	planes[1][3] /= length;

	// Left plane
	planes[2][0] = viewProjMatrix[0][3] + viewProjMatrix[0][0];
	planes[2][1] = viewProjMatrix[1][3] + viewProjMatrix[1][0];
	planes[2][2] = viewProjMatrix[2][3] + viewProjMatrix[2][0];
	planes[2][3] = viewProjMatrix[3][3] + viewProjMatrix[3][0];

	length = sqrtf(planes[2][0] * planes[2][0] + planes[2][1] * planes[2][1] + planes[2][2] * planes[2][2]);
	planes[2][0] /= length;
	planes[2][1] /= length;
	planes[2][2] /= length;
	planes[2][3] /= length;

	// Right plane
	planes[3][0] = viewProjMatrix[0][3] - viewProjMatrix[0][0];
	planes[3][1] = viewProjMatrix[1][3] - viewProjMatrix[1][0];
	planes[3][2] = viewProjMatrix[2][3] - viewProjMatrix[2][0];
	planes[3][3] = viewProjMatrix[3][3] - viewProjMatrix[3][0];

	length = sqrtf(planes[3][0] * planes[3][0] + planes[3][1] * planes[3][1] + planes[3][2] * planes[3][2]);
	planes[3][0] /= length;
	planes[3][1] /= length;
	planes[3][2] /= length;
	planes[3][3] /= length;

	// Bottom plane
	planes[4][0] = viewProjMatrix[0][3] + viewProjMatrix[0][1];
	planes[4][1] = viewProjMatrix[1][3] + viewProjMatrix[1][1];
	planes[4][2] = viewProjMatrix[2][3] + viewProjMatrix[2][1];
	planes[4][3] = viewProjMatrix[3][3] + viewProjMatrix[3][1];

	length = sqrtf(planes[4][0] * planes[4][0] + planes[4][1] * planes[4][1] + planes[4][2] * planes[4][2]);
	planes[4][0] /= length;
	planes[4][1] /= length;
	planes[4][2] /= length;
	planes[4][3] /= length;

	// Top plane
	planes[5][0] = viewProjMatrix[0][3] - viewProjMatrix[0][1];
	planes[5][1] = viewProjMatrix[1][3] - viewProjMatrix[1][1];
	planes[5][2] = viewProjMatrix[2][3] - viewProjMatrix[2][1];
	planes[5][3] = viewProjMatrix[3][3] - viewProjMatrix[3][1];

	length = sqrtf(planes[5][0] * planes[5][0] + planes[5][1] * planes[5][1] + planes[5][2] * planes[5][2]);
	planes[5][0] /= length;
	planes[5][1] /= length;
	planes[5][2] /= length;
	planes[5][3] /= length;
}

bool FrustumCuller::IsInsideFrustum(Model * model)
{
	float distance;
	glm::vec3 position = model->GetPosition();
	float radius = model->GetFrustumCullRadius();

	for (int i = 0; i < 6; i++)
	{
		distance = planes[i][0] * position.x + planes[i][1] * position.y + planes[i][2] * position.z + planes[i][3];
		if (distance <= -radius)
			return false;
	}

	return true;
}
