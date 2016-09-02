/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: TimeCycle.h                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <vector>
#include <string>
#include <glm.hpp>
#include "Skydome.h"
#include "Light.h"
#include "GameplayTimer.h"

#pragma once

class TimeCycle
{
	private:
		struct Entry
		{
			unsigned short hour, minute;
			glm::vec3 skyColor;
			glm::vec3 atmosphereColor;
			float skyHeight;
			glm::vec3 ambientColor;
			glm::vec3 diffuseColor;
			glm::vec3 specularColor;
			glm::vec3 lightDirection;
			float shadowStrength;
		};
		struct Weather
		{
			std::string weatherName;
			std::vector<Entry> entries;
		};
		std::vector<Weather> weatherList;
		unsigned short hour, minute;
		Weather currentWeather;
		Skydome * skydomePtr;
		Light * lightPtr;
		GameplayTimer * timer;
		bool freezeTime;
		float timePassSpeed;
	private:
		int ConvertToMinutes(unsigned short hour, unsigned short minutes);
		float ConvertToMixValue(unsigned short hour, unsigned short minutes, Entry first, Entry second);
	public:
		bool Init(Skydome * skydome, Light * light);
		void Update();
		void SetTime(unsigned short hour, unsigned short minute);
		void SetWeather(std::string weatherName);
		void FreezeTime(bool toggle);
		void SetTimePassSpeed(float timeInMs);
		unsigned short GetCurrentHour();
		unsigned short GetCurrentMinute();
};