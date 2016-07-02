/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Input.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#define KEYBOARD_KEY_ESCAPE 0x1B
#define KEYBOARD_KEY_F 0x46

class Input
{
	public:
		bool IsKeyPressed(int key);
};