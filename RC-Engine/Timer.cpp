/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Timer.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <Windows.h>

#include "Timer.h"

void Timer::Init()
{
	fpsCounter = fps = 1;
	fpsStartTime = timeGetTime();
}

void Timer::Update()
{
	fpsCounter++;
	if (timeGetTime() >= (fpsStartTime + 1000))
	{
		fps = fpsCounter;
		fpsCounter = 1;

		fpsStartTime = timeGetTime();
	}
}

int Timer::GetFPS()
{
	return fps;
}
