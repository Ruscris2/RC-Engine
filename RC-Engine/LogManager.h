/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: LogManager.h                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <iostream>
#include <fstream>
#include <string>

class LogManager
{
	private:
		std::ofstream file;
	public:
		bool Init();
		~LogManager();
		void AddMessage(std::string msg);
};
