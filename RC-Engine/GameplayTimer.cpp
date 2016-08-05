/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: GameplayTimer.cpp                                    |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <Windows.h>

#include "GameplayTimer.h"

GameplayTimer::GameplayTimer()
{
	timeStart = 0;
	timerRunning = false;
}

void GameplayTimer::StartTimer()
{
	if (timerRunning == false)
	{
		timeStart = timeGetTime();
		timerRunning = true;
	}
}

void GameplayTimer::ResetTimer()
{
	timerRunning = false;
}

bool GameplayTimer::TimePassed(float timeInMs)
{
	if (timeGetTime() > (timeStart + timeInMs))
		return true;

	return false;
}

bool GameplayTimer::IsTimerRunning()
{
	return timerRunning;
}
