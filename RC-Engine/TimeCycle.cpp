/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: TimeCycle.cpp                                        |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>
#include "TimeCycle.h"
#include "LogManager.h"

extern LogManager * gLogManager;

bool TimeCycle::Init(Skydome * skydome, Light * light)
{
	skydomePtr = skydome;
	lightPtr = light;
	timePassSpeed = 1000;

	std::ifstream file("data/timecycle.dat");
	if (!file.is_open())
	{
		gLogManager->AddMessage("ERROR: Missing timecycle.dat");
		return false;
	}

	std::string identifier, weatherName;
	while (!file.eof())
	{
		// Create new weather
		Weather weather;
		file >> identifier >> weatherName;
		weather.weatherName = weatherName;

		// Check if there is an entry
		file >> identifier;
		while (identifier != "end")
		{
			Entry entry;
			std::string attribute;
			
			// Check if there is an attribute
			file >> attribute;
			while (attribute != "endentry")
			{
				if (attribute == "time")
					file >> entry.hour >> entry.minute;
				else if (attribute == "skycolor")
					file >> entry.skyColor.r >> entry.skyColor.g >> entry.skyColor.b;
				else if (attribute == "atmcolor")
					file >> entry.atmosphereColor.r >> entry.atmosphereColor.g >> entry.atmosphereColor.b;
				else if (attribute == "skyheight")
					file >> entry.skyHeight;
				else if (attribute == "ambient")
					file >> entry.ambientColor.r >> entry.ambientColor.g >> entry.ambientColor.b;
				else if (attribute == "diffuse")
					file >> entry.diffuseColor.r >> entry.diffuseColor.g >> entry.diffuseColor.b;
				else if (attribute == "specular")
					file >> entry.specularColor.r >> entry.specularColor.g >> entry.specularColor.b;
				else if (attribute == "lightdir")
					file >> entry.lightDirection.x >> entry.lightDirection.y >> entry.lightDirection.z;
				else if (attribute == "shadow")
					file >> entry.shadowStrength;
				else
				{
					gLogManager->AddMessage("ERROR: Unknown attribute detected: " + attribute);
					return false;
				}

				file >> attribute;
			}

			// Add entry to current weather
			weather.entries.push_back(entry);

			file >> identifier;
		}

		// Add current weather to weather list
		weatherList.push_back(weather);
	}

	file.close();

	timer = new GameplayTimer();
	return true;
}

int TimeCycle::ConvertToMinutes(unsigned short hour, unsigned short minutes)
{
	return hour * 60 + minutes;
}

float TimeCycle::ConvertToMixValue(unsigned short hour, unsigned short minutes, Entry first, Entry second)
{
	int firstEntryInMinutes = ConvertToMinutes(first.hour, first.minute);
	int secondEntryInMinutes = ConvertToMinutes(second.hour, second.minute);
	int currentTimeInMinutes = ConvertToMinutes(hour, minute);

	if (firstEntryInMinutes == secondEntryInMinutes)
		return 1.0f;

	int interval = secondEntryInMinutes - firstEntryInMinutes;
	currentTimeInMinutes -= firstEntryInMinutes;

	return (float)currentTimeInMinutes / interval;
}

void TimeCycle::Update()
{
	// Advance time
	if (!freezeTime)
	{
		timer->StartTimer();

		if (timer->TimePassed(timePassSpeed))
		{
			minute++;
			if (minute == 60)
			{
				hour++;
				minute = 0;
			}
			if (hour == 24)
				hour = 0;

			timer->ResetTimer();
		}
	}

	// Find two entries that are closest to the current hour and minute
	Entry first, second;
	unsigned int entryIndex;
	int timeDifference;

	int currentTimeInMinutes = ConvertToMinutes(hour, minute);

	// Find first entry (entry time <= current time)
	timeDifference = INT_MAX;
	for (unsigned int i = 0; i < currentWeather.entries.size(); i++)
	{
		int entryTimeInMinutes = ConvertToMinutes(currentWeather.entries[i].hour, currentWeather.entries[i].minute);
		if (currentTimeInMinutes - entryTimeInMinutes >= 0 && currentTimeInMinutes - entryTimeInMinutes < timeDifference)
		{
			timeDifference = currentTimeInMinutes - entryTimeInMinutes;
			entryIndex = i;
		}
	}

	first = currentWeather.entries[entryIndex];

	// Find second entry (entry time >= current time)
	timeDifference = INT_MIN;
	for (unsigned int i = 0; i < currentWeather.entries.size(); i++)
	{
		int entryTimeInMinutes = ConvertToMinutes(currentWeather.entries[i].hour, currentWeather.entries[i].minute);
		if (currentTimeInMinutes - entryTimeInMinutes <= 0 && currentTimeInMinutes - entryTimeInMinutes > timeDifference)
		{
			timeDifference = currentTimeInMinutes - entryTimeInMinutes;
			entryIndex = i;
		}
	}

	second = currentWeather.entries[entryIndex];
	
	// Mix the values between the two entries
	float mixValue = ConvertToMixValue(hour, minute, first, second);

	glm::vec3 skyColor = glm::mix(first.skyColor, second.skyColor, mixValue);
	glm::vec3 atmosphereColor = glm::mix(first.atmosphereColor, second.atmosphereColor, mixValue);
	float skyHeight = glm::mix(first.skyHeight, second.skyHeight, mixValue);
	glm::vec3 ambientColor = glm::mix(first.ambientColor, second.ambientColor, mixValue);
	glm::vec3 diffuseColor = glm::mix(first.diffuseColor, second.diffuseColor, mixValue);
	glm::vec3 specularColor = glm::mix(first.specularColor, second.specularColor, mixValue);
	glm::vec3 lightDir = glm::mix(first.lightDirection, second.lightDirection, mixValue);
	float shadowStrength = glm::mix(first.shadowStrength, second.shadowStrength, mixValue);

	// Apply the values to light and skydome
	skydomePtr->SetSkyColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
	skydomePtr->SetAtmosphereColor(atmosphereColor.r, atmosphereColor.g, atmosphereColor.b, 1.0f);
	skydomePtr->SetAtmosphereHeight(skyHeight);
	lightPtr->SetAmbientColor(ambientColor.r, ambientColor.g, ambientColor.b, 1.0f);
	lightPtr->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f);
	lightPtr->SetSpecularColor(specularColor.r, specularColor.g, specularColor.b, 1.0f);
	lightPtr->SetLightDirection(lightDir.x, lightDir.y, lightDir.z);
	lightPtr->SetShadowStrength(shadowStrength);
}

void TimeCycle::SetTime(unsigned short hour, unsigned short minute)
{
	if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59)
	{
		this->hour = hour;
		this->minute = minute;
	}
	else
		gLogManager->AddMessage("WARNING: Unsupported timecycle time!");
}

void TimeCycle::SetWeather(std::string weatherName)
{
	for(unsigned int i = 0; i < weatherList.size(); i++)
		if (weatherList[i].weatherName == weatherName)
		{
			currentWeather = weatherList[i];
			return;
		}

	gLogManager->AddMessage("WARNING: Unsupported timecycle weather!");
}

void TimeCycle::FreezeTime(bool toggle)
{
	freezeTime = toggle;
}

void TimeCycle::SetTimePassSpeed(float timeInMs)
{
	timePassSpeed = timeInMs;
}

unsigned short TimeCycle::GetCurrentHour()
{
	return hour;
}

unsigned short TimeCycle::GetCurrentMinute()
{
	return minute;
}
