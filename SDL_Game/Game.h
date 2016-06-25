#ifndef GAME_H
#define GAME_H

#include "defs.h"

/*
  IMPORTANT:

  This file is our game API that will be shared between platform specific code.
*/

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

enum EntityType
{
    ENTITY_NULL,
    ENTITY_PLAYER,
    ENTITY_WALL,
};

struct Entity
{
    EntityType type;
    r32 xPos;
    r32 yPos;
};

struct GameMemory
{
    void* memoryBlock;
    u32 blockSize;
};

struct GameState
{
    u32 screenW;            //4
    u32 screenH;            //4
    SDL_Renderer* renderer; //4

    r32 dt;                 //4

    u32 entityCount;
    Entity* player;
    
    bool isInitialized;
}; //24 bytes

#define UPDATE_RENDER(name) void name(GameState* gameState, GameController* input, GameMemory* memory)
typedef UPDATE_RENDER(update_render);

#endif
