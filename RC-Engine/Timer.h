/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Timer.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <Windows.h>

class Timer
{
	private:
		INT64 frequency, startTime;
		float ticksPerMs, delta;
		int fps, fpsCounter;
		unsigned long fpsStartTime;
	public:
		bool Init();
		void Update();
		int GetFPS();
		float GetDelta();
};