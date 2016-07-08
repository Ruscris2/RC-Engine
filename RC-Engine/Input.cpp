/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Input.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <Windows.h>

#include "Input.h"

Input::Input()
{
	cursorRelativeX = cursorRelativeY = 0;
	prevCursorX = prevCursorY = INT_MAX;
	
	keyStateValues.resize(KEY_STATE_COUNT);
	keyStateValues[KEYBOARD_KEY_ESCAPE] = 0x1B;
	keyStateValues[KEYBOARD_KEY_F] = 0x46;
	keyStateValues[KEYBOARD_KEY_W] = 0x57;
	keyStateValues[KEYBOARD_KEY_A] = 0x41;
	keyStateValues[KEYBOARD_KEY_S] = 0x53;
	keyStateValues[KEYBOARD_KEY_D] = 0x44;
	keyStateValues[MOUSE_LEFTBUTTON] = 0x01;
	keyStateValues[KEYBOARD_KEY_1] = 0x31;
	keyStateValues[KEYBOARD_KEY_2] = 0x32;
	keyStateValues[KEYBOARD_KEY_3] = 0x33;
	keyStateValues[KEYBOARD_KEY_4] = 0x34;
	keyStateValues[KEYBOARD_KEY_5] = 0x35;

	prevFrameState = new bool[KEY_STATE_COUNT];
	currentFrameState = new bool[KEY_STATE_COUNT];
	memset(prevFrameState, 0, sizeof(bool) * KEY_STATE_COUNT);
	memset(currentFrameState, 0, sizeof(bool) * KEY_STATE_COUNT);
}

Input::~Input()
{
	delete[] currentFrameState;
	delete[] prevFrameState;
}

bool Input::IsKeyPressed(int key)
{
	return currentFrameState[key];
}

bool Input::WasKeyPressed(int key)
{
	return (prevFrameState[key] && !currentFrameState[key]);
}

void Input::Update()
{
	POINT p;
	GetCursorPos(&p);
	
	if (prevCursorX == INT_MAX)
	{
		prevCursorX = (int)p.x;
		prevCursorY = (int)p.y;
		return;
	}

	cursorRelativeX = (int)p.x - prevCursorX;
	cursorRelativeY = (int)p.y - prevCursorY;
	prevCursorX = (int)p.x;
	prevCursorY = (int)p.y;

	for (unsigned int i = 0; i < keyStateValues.size(); i++)
	{
		prevFrameState[i] = currentFrameState[i];
		if (GetAsyncKeyState(keyStateValues[i]))
			currentFrameState[i] = true;
		else
			currentFrameState[i] = false;
	}
}

int Input::GetCursorRelativeX()
{
	return cursorRelativeX;
}

int Input::GetCursorRelativeY()
{
	return cursorRelativeY;
}
