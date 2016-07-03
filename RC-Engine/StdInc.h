/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: StdInc.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <Windows.h>

#define PROGRAM_NAME "RC-Engine"
#define PROGRAM_VERSION "0.1"
#define PROGRAM_BUILD "build 6"
#define PROGRAM_IDENTIFIER PROGRAM_NAME " " PROGRAM_VERSION " " PROGRAM_BUILD

#define THROW_ERROR() { MessageBox(NULL, "An error has been detected! Check log.txt for more information!", "ERROR!", MB_OK | MB_ICONERROR); ExitProcess(1); }
#define SAFE_DELETE(mem) { if(mem) { delete mem; mem = NULL; } }
#define SAFE_UNLOAD(mem, ...) { if(mem) { mem->Unload(__VA_ARGS__); delete mem; mem = NULL; } }