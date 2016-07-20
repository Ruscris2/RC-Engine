/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Timer.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Timer.h"

bool Timer::Init()
{
	benchmarkResult = 0.0f;
	fpsCounter = fps = 1;
	fpsStartTime = timeGetTime();

	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	if (frequency == 0)
		return false;

	ticksPerMs = (float)(frequency / 1000);

	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	return true;
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

	INT64 currentTime;
	float timeDifference;

	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	timeDifference = (float)(currentTime - startTime);
	delta = timeDifference / ticksPerMs;
	startTime = currentTime;
}

int Timer::GetFPS()
{
	return fps;
}

float Timer::GetDelta()
{
	return delta;
}

void Timer::BenchmarkCodeStart()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&benchmarkStartTime);
}

void Timer::BenchmarkCodeEnd()
{
	INT64 currentTime;
	float timeDifference;

	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	timeDifference = (float)(currentTime - benchmarkStartTime);
	benchmarkResult = timeDifference / ticksPerMs;
}

float Timer::GetBenchmarkResult()
{
	return benchmarkResult;
}
