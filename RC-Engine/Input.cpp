/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Input.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "Input.h"

Input::Input()
{
	inputHandler_cursorRelatives_updated = false;
	inputHandler_setKeyUp_updated = false;

	cursorRelativeX = cursorRelativeY = 0;
	
	keyStateValues.resize(KEY_STATE_COUNT);
	keyStateValues[KEYBOARD_KEY_ESCAPE] = 0x1B;
	keyStateValues[KEYBOARD_KEY_F] = 0x46;
	keyStateValues[KEYBOARD_KEY_W] = 0x57;
	keyStateValues[KEYBOARD_KEY_A] = 0x41;
	keyStateValues[KEYBOARD_KEY_S] = 0x53;
	keyStateValues[KEYBOARD_KEY_D] = 0x44;
	keyStateValues[KEYBOARD_KEY_1] = 0x31;
	keyStateValues[KEYBOARD_KEY_2] = 0x32;
	keyStateValues[KEYBOARD_KEY_3] = 0x33;
	keyStateValues[KEYBOARD_KEY_4] = 0x34;
	keyStateValues[KEYBOARD_KEY_5] = 0x35;
	keyStateValues[KEYBOARD_KEY_6] = 0x36;
	keyStateValues[KEYBOARD_KEY_7] = 0x37;
	keyStateValues[KEYBOARD_KEY_8] = 0x38;
	keyStateValues[KEYBOARD_KEY_9] = 0x39;
	keyStateValues[KEYBOARD_KEY_0] = 0x30;
	keyStateValues[KEYBOARD_KEY_Q] = 0x51;
	keyStateValues[KEYBOARD_KEY_E] = 0x45;
	keyStateValues[KEYBOARD_KEY_R] = 0x52;
	keyStateValues[KEYBOARD_KEY_T] = 0x54;
	keyStateValues[KEYBOARD_KEY_Y] = 0x59;
	keyStateValues[KEYBOARD_KEY_U] = 0x55;
	keyStateValues[KEYBOARD_KEY_I] = 0x49;
	keyStateValues[KEYBOARD_KEY_O] = 0x4F;
	keyStateValues[KEYBOARD_KEY_P] = 0x50;
	keyStateValues[KEYBOARD_KEY_G] = 0x47;
	keyStateValues[KEYBOARD_KEY_H] = 0x48;
	keyStateValues[KEYBOARD_KEY_J] = 0x4A;
	keyStateValues[KEYBOARD_KEY_K] = 0x4B;
	keyStateValues[KEYBOARD_KEY_L] = 0x4C;
	keyStateValues[KEYBOARD_KEY_Z] = 0x5A;
	keyStateValues[KEYBOARD_KEY_X] = 0x58;
	keyStateValues[KEYBOARD_KEY_C] = 0x43;
	keyStateValues[KEYBOARD_KEY_V] = 0x56;
	keyStateValues[KEYBOARD_KEY_B] = 0x42;
	keyStateValues[KEYBOARD_KEY_N] = 0x4E;
	keyStateValues[KEYBOARD_KEY_M] = 0x4D;
	keyStateValues[KEYBOARD_KEY_SPACE] = 0x20;
	keyStateValues[KEYBOARD_KEY_SHIFT] = 0x10;
	memset(wasKeyPressed, 0, sizeof(bool) * 256);
	memset(keyboardState, 0, sizeof(bool) * 256);
}

Input::~Input()
{

}

bool Input::Init(HWND hwnd)
{
	RAWINPUTDEVICE rid[2];

	// Mouse raw input description
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
	rid[0].dwFlags = 0;
	rid[0].hwndTarget = hwnd;

	// Keyboard raw input description
	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x06;
	rid[1].dwFlags = 0;
	rid[1].hwndTarget = hwnd;

	if (RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)) == FALSE)
		return false;

	return true;
}

void Input::InputHandler_SetCursorRelatives(int x, int y)
{
	cursorRelativeX = x;
	cursorRelativeY = y;
	inputHandler_cursorRelatives_updated = true;
}

void Input::InputHandler_SetKeyDown(short vKey)
{
	keyboardState[vKey] = true;
}

void Input::InputHandler_SetKeyUp(short vKey)
{
	if(keyboardState[vKey] == true)
		wasKeyPressed[vKey] = true;

	keyboardState[vKey] = false;
	inputHandler_setKeyUp_updated = true;
}

bool Input::IsKeyPressed(int key)
{
	return keyboardState[keyStateValues[key]];
}

bool Input::WasKeyPressed(int key)
{
	return wasKeyPressed[keyStateValues[key]];
}

void Input::Update()
{
	if (inputHandler_cursorRelatives_updated == true)
		inputHandler_cursorRelatives_updated = false;
	else
	{
		cursorRelativeX = 0;
		cursorRelativeY = 0;
	}

	if (inputHandler_setKeyUp_updated == true)
		inputHandler_setKeyUp_updated = false;
	else
		memset(wasKeyPressed, 0, sizeof(bool) * 256);
}

int Input::GetCursorRelativeX()
{
	return cursorRelativeX;
}

int Input::GetCursorRelativeY()
{
	return cursorRelativeY;
}
