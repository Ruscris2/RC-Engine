/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Timer.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

class Timer
{
	private:
		int fps, fpsCounter;
		unsigned long fpsStartTime;
	public:
		void Init();
		void Update();
		int GetFPS();
};