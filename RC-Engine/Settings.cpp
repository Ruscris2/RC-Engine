/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Settings.cpp                                         |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>
#include <limits>

#include "Settings.h"

Settings::Settings()
{
	windowWidth = 800;
	windowHeight = 600;
}

bool Settings::ReadSettings()
{
	std::ifstream file("data/settings.cfg");
	if (!file.is_open())
	{
		Settings();
		return false;
	}

	std::string identifier;
	while (!file.eof())
	{
		file >> identifier;

		// Skip comment
		if (identifier[0] == '/' && identifier[1] == '/')
		{
			file.ignore(std::numeric_limits<std::streamsize>::max(), file.widen('\n'));
			continue;
		}

		if (identifier == "width")
			file >> windowWidth;
		else if (identifier == "height")
			file >> windowHeight;
		else
		{
			Settings();
			return false;
		}
	}
	return true;
}

int Settings::GetWindowWidth()
{
	return windowWidth;
}

int Settings::GetWindowHeight()
{
	return windowHeight;
}

