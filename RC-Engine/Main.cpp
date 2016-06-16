/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Main.cpp                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <iostream>

#include "StdInc.h"
#include "LogManager.h"
#include "WinWindow.h"
#include "Settings.h"

LRESULT CALLBACK WinWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

LogManager * gLogManager;
Settings * gSettings;

bool gProgramRunning = true;

int main()
{
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	gLogManager = new LogManager();
	if (!gLogManager->Init())
	{
		std::cout << "ERROR: Failed to init LogManager!" << std::endl;
		return 1;
	}
	gLogManager->AddMessage(std::string(PROGRAM_IDENTIFIER) + " started!");

	gSettings = new Settings();
	if (!gSettings->ReadSettings())
		gLogManager->AddMessage("WARNING: Couldn't read settings.cfg! Using defaults...");
	else
		gLogManager->AddMessage("SUCESS: Loaded settings.cfg!");

	WinWindow * window = new WinWindow();
	if (!window->Create(PROGRAM_NAME, gSettings->GetWindowWidth(), gSettings->GetWindowHeight(), 100, 100, WinWindowProc))
	{
		gLogManager->AddMessage("ERROR: Failed to init render window!");
		THROW_ERROR();
	}

	MSG msg;
	while (gProgramRunning)
	{
		if (PeekMessage(&msg, window->GetHWND(), 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	gLogManager->AddMessage("Unloading...");
	SAFE_DELETE(window);
	SAFE_DELETE(gLogManager);
	
	return 0;
}

LRESULT CALLBACK WinWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_CLOSE:
			gProgramRunning = false;
		break;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}