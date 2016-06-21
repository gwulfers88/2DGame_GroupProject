#ifndef SYS_WIN_H
#define SYS_WIN_H

#include <Windows.h>
#include "defs.h"
#include <VLD\vld.h>

//Linking to our libs
#pragma comment (lib, "SDL2.lib")
#pragma comment (lib, "SDL2main.lib")

bool			isRunning		= false;
i32				wndWidth		= 1024;
i32				wndHeight		= 768;
i8				wndTitle[]		= "SDL Game Project";
i8				wndClassName[8] = "gj2016";

SDL_Window*		sdlWindow		= 0;
SDL_Renderer*	renderer		= 0;

void* gameMemoryBlock = 0;

void stringCopy(i8* dest, const i8* src)
{
	while(*src)
	{
		*dest++ = *src++;
	}

	*dest++ = 0;
}

#endif