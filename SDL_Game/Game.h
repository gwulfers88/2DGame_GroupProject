#ifndef GAME_H
#define GAME_H

#include "defs.h"

struct GameButton
{
    bool isDown;
};

struct GameController
{
    GameButton actionLeft;
    GameButton actionRight;
    GameButton actionUp;
    GameButton actionDown;

    GameButton start;
    GameButton back;
};

struct GameState
{
    void* memoryBlock;      //4
    u32 screenW;            //4
    u32 screenH;            //4
    u32 blockSize;          //4
    SDL_Renderer* renderer; //4
    r32 dt;                 //4

    bool isInitialized;
}; //24 bytes

#define UPDATE_RENDER(name) void name(GameState* gameState, GameController* input)
typedef UPDATE_RENDER(update_render);

#endif
