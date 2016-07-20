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
	if (!gInput->Init(window->GetHWND()))
	{
		gLogManager->AddMessage("ERROR: Failed to init input interface!");
		return false;
	}

	//Vulkan interface
	VulkanInterface * vulkan = new VulkanInterface();
	if (!vulkan->Init(window->GetHWND()))
	{
		gLogManager->AddMessage("ERROR: Failed to init vulkan interface!");
		THROW_ERROR();
	}
	gLogManager->AddMessage("SUCCESS: Vulkan interface initialized!");

	// Timer
	gTimer = new Timer();
	if (!gTimer->Init())
	{
		gLogManager->AddMessage("ERROR: Failed to init timer!");
		return false;
	}

	// Scene manager
	SceneManager * sceneManager = new SceneManager();
	if (!sceneManager->Init(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init scene manager!");
		THROW_ERROR();
	}

	// Main loop
	MSG msg;
	while (gProgramRunning)
	{
		while (PeekMessage(&msg, window->GetHWND(), 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
			if(msg.message == WM_INPUT)
			{
				UINT dwSize;
				GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
				LPBYTE lpb = new BYTE[dwSize];

				GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

				RAWINPUT * raw = (RAWINPUT*)lpb;

				if (raw->header.dwType == RIM_TYPEMOUSE)
					gInput->InputHandler_SetCursorRelatives((int)raw->data.mouse.lLastX, (int)raw->data.mouse.lLastY);
				else if (raw->header.dwType = RIM_TYPEKEYBOARD)
				{
					if (raw->data.keyboard.Flags == RI_KEY_BREAK)
						gInput->InputHandler_SetKeyUp(raw->data.keyboard.VKey);
					if (raw->data.keyboard.Flags == RI_KEY_MAKE)
						gInput->InputHandler_SetKeyDown(raw->data.keyboard.VKey);
				}
			}
		}
		gTimer->Update();
		gInput->Update();
		sceneManager->Render(vulkan);

		if (gInput->WasKeyPressed(KEYBOARD_KEY_ESCAPE))
			gProgramRunning = false;
		if (gInput->WasKeyPressed(KEYBOARD_KEY_F))
		{
			char msg[64];
			sprintf(msg, "FPS: %d FRAME TIME: %f BENCH TIME: %f", gTimer->GetFPS(), gTimer->GetDelta(), gTimer->GetBenchmarkResult());
			gLogManager->AddMessage(msg);
		}

	}

	vkDeviceWaitIdle(vulkan->GetVulkanDevice()->GetDevice());

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
		case WM_SETFOCUS:
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT p1 = { rect.left, rect.top };
			POINT p2 = { rect.right, rect.bottom };
			ClientToScreen(hwnd, &p1);
			ClientToScreen(hwnd, &p2);
			SetRect(&rect, p1.x, p1.y, p2.x, p2.y);
			ClipCursor(&rect);
			ShowCursor(FALSE);

			if (gSettings->GetFullscreenMode())
			{
				DEVMODE dm;
				dm.dmSize = sizeof(DEVMODE);
				dm.dmDriverExtra = 0;

				EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

				dm.dmPelsWidth = gSettings->GetWindowWidth();
				dm.dmPelsHeight = gSettings->GetWindowHeight();

				ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);
			}
		}
		break;
		case WM_KILLFOCUS:
		{
			ClipCursor(NULL);
			ShowCursor(TRUE);

			if (gSettings->GetFullscreenMode())
			{
				ChangeDisplaySettings(NULL, 0);
				ShowWindow(hwnd, SW_MINIMIZE);
			}
		}
		break;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}