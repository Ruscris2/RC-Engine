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
#include "VulkanInterface.h"
#include "Input.h"
#include "Timer.h"
#include "SceneManager.h"

LRESULT CALLBACK WinWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

LogManager * gLogManager;
Settings * gSettings;
Input * gInput;
Timer * gTimer;

bool gProgramRunning = true;

int main()
{
	// Console window
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// Log manager
	gLogManager = new LogManager();
	if (!gLogManager->Init())
	{
		std::cout << "ERROR: Failed to init LogManager!" << std::endl;
		return 1;
	}
	gLogManager->AddMessage(std::string(PROGRAM_IDENTIFIER) + " started!");

	// Settings
	gSettings = new Settings();
	if (!gSettings->ReadSettings())
		gLogManager->AddMessage("WARNING: Couldn't read settings.cfg! Using defaults...");
	else
		gLogManager->AddMessage("SUCCESS: Loaded settings.cfg!");

	// Render window
	WinWindow * window = new WinWindow();
	if (!window->Create(PROGRAM_NAME, gSettings->GetWindowWidth(), gSettings->GetWindowHeight(), 500, 50, WinWindowProc))
	{
		gLogManager->AddMessage("ERROR: Failed to init render window!");
		THROW_ERROR();
	}
	
	// Input
	gInput = new Input();

	//Vulkan interface
	VulkanInterface * vulkan = new VulkanInterface();
	if (!vulkan->Init(window->GetHWND()))
	{
		gLogManager->AddMessage("ERROR: Failed to init vulkan interface!");
		THROW_ERROR();
	}
	gLogManager->AddMessage("SUCCESS: Vulkan interface initialized!");

	// Scene manager
	SceneManager * sceneManager = new SceneManager();
	if (!sceneManager->Init(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init scene manager!");
		THROW_ERROR();
	}

	// Timer
	gTimer = new Timer();
	if (!gTimer->Init())
	{
		gLogManager->AddMessage("ERROR: Failed to init timer!");
		return false;
	}

	// Main loop
	MSG msg;
	while (gProgramRunning)
	{
		if (PeekMessage(&msg, window->GetHWND(), 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		gTimer->Update();
		gInput->Update();
		sceneManager->Render(vulkan);

		if (gInput->WasKeyPressed(KEYBOARD_KEY_ESCAPE))
			gProgramRunning = false;
		if (gInput->WasKeyPressed(KEYBOARD_KEY_F))
		{
			char msg[16];
			sprintf(msg, "FPS: %d", gTimer->GetFPS());
			gLogManager->AddMessage(msg);
		}
	}

	gLogManager->AddMessage("Unloading...");
	SAFE_DELETE(gTimer);
	SAFE_UNLOAD(sceneManager, vulkan);
	SAFE_DELETE(vulkan);
	SAFE_DELETE(gInput);
	SAFE_DELETE(window);
	SAFE_DELETE(gSettings);
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