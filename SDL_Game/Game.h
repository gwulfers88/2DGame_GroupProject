#ifndef GAME_H
#define GAME_H

#include "defs.h"

/*
  IMPORTANT:

  This file is our game API that will be shared between platform specific code.
*/

struct ReadFileResult
{
    void* data;
    u32 fileSize;
};

#define READ_ENTIRE_FILE(name) ReadFileResult name(char* filename)
typedef READ_ENTIRE_FILE(read_entire_file);

#define WRITE_ENTIRE_FILE(name) void name(char* filename, ReadFileResult* fileResult)
typedef WRITE_ENTIRE_FILE(write_entire_file);

#define FREE_FILE(name) void name(ReadFileResult* fileResult)
typedef FREE_FILE(free_file);

#define Assert(expression) if(!expression){ *(int*)0 = 0; }

struct GameButton
{
    bool isDown;
};

struct GameController
{
    r32 dt;
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
    void* permanentBlock;
    void* transientBlock;

    u32 permanentSize;
    u32 transientSize;
    u32 totalSize;
    
    bool isInitialized;

    read_entire_file* readEntireFile;
    write_entire_file* writeEntireFile;
    free_file* freeFile;
};

//TODO: Is this what we want to call it?
struct Render
{
    u32 screenW;
    u32 screenH;
    SDL_Renderer* renderer;
};

struct TileMap
{
    u32 countX;
    u32 countY;
    u32 width;
    u32 height;
    u32 offsetX;
    u32 offsetY;
    u32 *tiles;
};

struct GameState
{
    u32 entityCount;
    Entity player;
};

#define UPDATE_RENDER(name) void name(GameMemory* memory, GameController* input, Render* render)
typedef UPDATE_RENDER(update_render);

#endif
