/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Physics.cpp                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "Physics.h"
#include "StdInc.h"
#include "Timer.h"

extern Timer * gTimer;

Physics::Physics()
{
	broadphase = NULL;
	collisionConfiguration = NULL;
	dispatcher = NULL;
	solver = NULL;
	dynamicsWorld = NULL;
}

Physics::~Physics()
{
	SAFE_DELETE(dynamicsWorld);
	SAFE_DELETE(solver);
	SAFE_DELETE(dispatcher);
	SAFE_DELETE(collisionConfiguration);
	SAFE_DELETE(broadphase);
}

bool Physics::Init()
{
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));
	
	return true;
}

void Physics::Update()
{
	dynamicsWorld->stepSimulation(gTimer->GetDelta() / 1000.0f, 0);
}

btDiscreteDynamicsWorld * Physics::GetDynamicsWorld()
{
	return dynamicsWorld;
}
