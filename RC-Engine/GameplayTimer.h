/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: GameplayTimer.h                                      |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#pragma once

class GameplayTimer
{
	private:
		unsigned long timeStart;
		bool timerRunning;
	public:
		GameplayTimer();

		void StartTimer();
		void ResetTimer();
		bool TimePassed(float timeInMs);
		bool IsTimerRunning();
		float GetTimeProgress();
};