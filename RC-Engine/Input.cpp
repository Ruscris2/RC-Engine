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
}

bool Input::IsKeyPressed(int key)
{
	if (GetAsyncKeyState(key))
		return true;

	return false;
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
}

int Input::GetCursorRelativeX()
{
	return cursorRelativeX;
}

int Input::GetCursorRelativeY()
{
	return cursorRelativeY;
}
