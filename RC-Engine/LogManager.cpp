/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: LogManager.cpp                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <time.h>

#include "LogManager.h"

bool LogManager::Init()
{
	file.open("log.txt");
	if (file.is_open() == false)
		return false;

	return true;
}

LogManager::~LogManager()
{
	file.close();
}

void LogManager::AddMessage(std::string msg)
{
	time_t t = time(NULL);
	struct tm * now = localtime(&t);

	std::cout << '[' << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << "] " << msg << std::endl;
	file << '[' << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << "] " << msg << std::endl;
}

void LogManager::PrintVector(glm::vec2 vec)
{
	char msg[64];
	sprintf(msg, "X: %f Y: %f", vec.x, vec.y);
	AddMessage(msg);
}

void LogManager::PrintVector(glm::vec3 vec)
{
	char msg[64];
	sprintf(msg, "X: %f Y: %f Z: %f", vec.x, vec.y, vec.z);
	AddMessage(msg);
}

void LogManager::PrintVector(glm::vec4 vec)
{
	char msg[64];
	sprintf(msg, "X: %f Y: %f Z: %f W: %f", vec.x, vec.y, vec.z, vec.w);
	AddMessage(msg);
}

void LogManager::PrintValue(float val)
{
	char msg[32];
	sprintf(msg, "VAL: %f", val);
	AddMessage(msg);
}
