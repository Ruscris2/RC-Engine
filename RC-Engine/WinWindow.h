/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: WinWindow.h                                          |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <string>
#include <Windows.h>

class WinWindow
{
	private:
		HWND hwnd;
	public:
		WinWindow();
		~WinWindow();

		bool Create(std::string windowName, int windowWidth, int windowHeight, int windowPosX, int windowPosY, WNDPROC windowProc);
		HWND GetHWND();
};