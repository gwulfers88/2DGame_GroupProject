#ifndef SYS_WIN_H
#define SYS_WIN_H

#include <Windows.h>
#include "Game.h"
#include "defs.h"

bool            isRunning       = false;
i32             wndWidth        = 1024;
i32             wndHeight       = 768;
i8              wndTitle[]      = "SDL Game Project";
i8              wndClassName[8] = "gj2016";

SDL_Window*     sdlWindow       = 0;
SDL_Renderer*   renderer        = 0;

void* gameMemoryBlock = 0;

struct Win32State
{
    HANDLE recordingHandle;
    u32 recordingIndex;

    HANDLE playbackHandle;
    u32 playbackIndex;

    void* memoryBlock;
    u32 memorySize;
};

struct ReadFileResult
{
    void* data;
    u32 fileSize;
};

struct GameCodeDLL
{
    HMODULE codeLib;
    update_render* UpdateRender;
    bool isValid;
    FILETIME lastWriteTime;
};

//TODO(George): MAKE A FILE FOR THIS SHIT
void stringCopy(i8* dest, const i8* src)
{
    while(*src)
    {
        *dest++ = *src++;
    }
    *dest++ = 0;
}

#endif
