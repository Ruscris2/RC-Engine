/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Input.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <Windows.h>

#include "Input.h"

bool Input::IsKeyPressed(int key)
{
	if (GetAsyncKeyState(key))
		return true;

	return false;
}
