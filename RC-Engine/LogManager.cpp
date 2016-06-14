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
