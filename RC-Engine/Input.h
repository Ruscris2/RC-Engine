/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Input.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <Windows.h>
#include <vector>

enum KEY_STATES
{
	KEYBOARD_KEY_ESCAPE,
	KEYBOARD_KEY_F,
	KEYBOARD_KEY_W,
	KEYBOARD_KEY_A,
	KEYBOARD_KEY_S,
	KEYBOARD_KEY_D,
	KEYBOARD_KEY_1,
	KEYBOARD_KEY_2,
	KEYBOARD_KEY_3,
	KEYBOARD_KEY_4,
	KEYBOARD_KEY_5,
	KEYBOARD_KEY_6,
	KEYBOARD_KEY_7,
	KEYBOARD_KEY_8,
	KEYBOARD_KEY_SHIFT,
	KEY_STATE_COUNT
};

class Input
{
	private:
		int cursorRelativeX, cursorRelativeY;
		std::vector<int> keyStateValues;
		bool wasKeyPressed[256];
		bool keyboardState[256];
		bool inputHandler_cursorRelatives_updated;
		bool inputHandler_setKeyUp_updated;
	public:
		Input();
		~Input();

		bool Init(HWND hwnd);
		void InputHandler_SetCursorRelatives(int x, int y);
		void InputHandler_SetKeyDown(short vKey);
		void InputHandler_SetKeyUp(short vKey);
		bool IsKeyPressed(int key);
		bool WasKeyPressed(int key);
		void Update();
		int GetCursorRelativeX();
		int GetCursorRelativeY();
};