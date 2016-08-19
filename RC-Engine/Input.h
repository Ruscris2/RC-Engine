/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Input.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <Windows.h>
#include <vector>

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
	KEYBOARD_KEY_9,
	KEYBOARD_KEY_0,
	KEYBOARD_KEY_Q,
	KEYBOARD_KEY_E,
	KEYBOARD_KEY_R,
	KEYBOARD_KEY_T,
	KEYBOARD_KEY_Y,
	KEYBOARD_KEY_U,
	KEYBOARD_KEY_I,
	KEYBOARD_KEY_O,
	KEYBOARD_KEY_P,
	KEYBOARD_KEY_G,
	KEYBOARD_KEY_H,
	KEYBOARD_KEY_J,
	KEYBOARD_KEY_K,
	KEYBOARD_KEY_L,
	KEYBOARD_KEY_Z,
	KEYBOARD_KEY_X,
	KEYBOARD_KEY_C,
	KEYBOARD_KEY_V,
	KEYBOARD_KEY_B,
	KEYBOARD_KEY_N,
	KEYBOARD_KEY_M,
	KEYBOARD_KEY_SPACE,
	KEYBOARD_KEY_SHIFT,
	KEY_STATE_COUNT
};