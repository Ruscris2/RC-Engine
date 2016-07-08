/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Input.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <vector>

enum KEY_STATES
{
	KEYBOARD_KEY_ESCAPE,
	KEYBOARD_KEY_F,
	KEYBOARD_KEY_W,
	KEYBOARD_KEY_A,
	KEYBOARD_KEY_S,
	KEYBOARD_KEY_D,
	MOUSE_LEFTBUTTON,
	KEYBOARD_KEY_1,
	KEYBOARD_KEY_2,
	KEYBOARD_KEY_3,
	KEYBOARD_KEY_4,
	KEYBOARD_KEY_5,
	KEY_STATE_COUNT
};

class Input
{
	private:
		int prevCursorX, prevCursorY;
		int cursorRelativeX, cursorRelativeY;
		std::vector<int> keyStateValues;
		bool * prevFrameState;
		bool * currentFrameState;
	public:
		Input();
		~Input();

		bool IsKeyPressed(int key);
		bool WasKeyPressed(int key);
		void Update();
		int GetCursorRelativeX();
		int GetCursorRelativeY();
};