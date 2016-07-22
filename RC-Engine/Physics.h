/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Physics.h                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <btBulletDynamicsCommon.h>

#pragma once

class Physics
{
	private:
		btBroadphaseInterface * broadphase;
		btDefaultCollisionConfiguration * collisionConfiguration;
		btCollisionDispatcher * dispatcher;
		btSequentialImpulseConstraintSolver * solver;
		btDiscreteDynamicsWorld * dynamicsWorld;
	public:
		Physics();
		~Physics();

		bool Init();
		void Update();
		btDiscreteDynamicsWorld * GetDynamicsWorld();
};
