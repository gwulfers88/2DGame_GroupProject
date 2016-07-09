#include "Game.h"
#include "vec2.h"

#include <random>
#include <time.h>

/*
    2D Top Down game ITS FINAL!!
    walking 4 ways (up down left right).
*/

// IMPORTANT: Moved Entity struct to Game.h

/*
  TODO(George): Things to do for monday!!!
  - Add the notion of a Camera.
  - Create Controller Input container.
  - Fix player movement.
  - Fix collision.
  - Start having multiple entities.
  - Shooting?
  - Sorting of sprites.
  - Make it possible to have multiple players in our game. (maybe 2 or 4 max).
*/

SDL_Texture* DEBUGLoadImage(Render* render, i8* filename)
{
    SDL_Texture* result = 0;
    SDL_Surface* surface = IMG_Load(filename);
    if(surface)
    {
        result = SDL_CreateTextureFromSurface(render->renderer, surface);
        SDL_FreeSurface(surface);
    }
    return result;
}

//0.0f - 1.0f 0 being dark and 1 bright
void SetDrawColor(Render* render, r32 r, r32 g, r32 b)
{
    SDL_SetRenderDrawColor(render->renderer, (u8)(r * 255),
                           (u8)(g * 255), (u8)(b * 255), 255);
}

void DrawRect(Render* render, i32 posX, i32 posY, i32 width, i32 height)
{            
    SDL_Rect rect = {};
    rect.x = posX;
    rect.y = posY;
    rect.w = width;
    rect.h = height;

    SDL_RenderFillRect(render->renderer, &rect);
}

TileChunk* GetTileChunk(TileMap* tileMap, u32 tileChunkX, u32 tileChunkY)
{
    TileChunk* tileChunk = 0;
    if(tileChunkX >= 0 && tileChunkX < tileMap->tileChunkCountX &&
       tileChunkY >= 0 && tileChunkY < tileMap->tileChunkCountY)
    {
        tileChunk = &tileMap->tileChunks[tileChunkY * tileMap->tileChunkCountX + tileChunkX];
    }
    return tileChunk;
}

i32 RoundR32ToI32(r32 value)
{
    i32 result = (value < 0.0f ? (i32)ceilf(value - 0.5f) : (i32)floorf(value + 0.5f));
    return result;
}

void RecanonicalizeCoords(TileMap* tileMap, u32* tile, r32* tileRel)
{
    i32 offset = RoundR32ToI32(*tileRel / tileMap->tileInUnits);
    *tile += offset;
    *tileRel -= offset * tileMap->tileInUnits;

    Assert((*tileRel >= -0.5f * tileMap->tileInUnits));
    Assert((*tileRel <= 0.5f * tileMap->tileInUnits));    
}

WorldPosition RecalcWorldPos(TileMap* tileMap, WorldPosition position)
{
    WorldPosition result = position;

    RecanonicalizeCoords(tileMap, &result.absTileX, &result.tileRel.x);
    RecanonicalizeCoords(tileMap, &result.absTileY, &result.tileRel.y);

    return result;
}

TileChunkPosition GetChunkPosition(TileMap* tileMap, u32 absTileX, u32 absTileY)
{
    TileChunkPosition result = {};

    result.tileChunkX = absTileX >> tileMap->chunkShift;
    result.tileChunkY = absTileY >> tileMap->chunkShift;
    result.tileX = absTileX & tileMap->chunkMask;
    result.tileY = absTileY & tileMap->chunkMask;
    
    return result;
}

u32 GetTileValue(TileMap* tileMap, TileChunk* tileChunk, u32 tileX, u32 tileY)
{
    u32 tileValue = 0;
    if(tileChunk && tileChunk->tiles)
    {
        tileValue = tileChunk->tiles[tileY * tileMap->chunkDims + tileX];
    }
    return tileValue;
}

u32 GetTileValue(TileMap* tileMap, u32 tileX, u32 tileY)
{
    TileChunkPosition chunkPos = GetChunkPosition(tileMap, tileX, tileY);
    TileChunk* tileChunk = GetTileChunk(tileMap, chunkPos.tileChunkX, chunkPos.tileChunkY);
    u32 tileValue = GetTileValue(tileMap, tileChunk, chunkPos.tileX, chunkPos.tileY);
    return tileValue;
}

bool IsTileMapLocEmpty(TileMap* tileMap, WorldPosition worldPos)
{
    u32 tileValue = GetTileValue(tileMap, worldPos.absTileX, worldPos.absTileY);
    bool empty = (tileValue == 1);
    return empty;
}

#define AllocStruct(pool, type) (type*) AllocMemory(pool, sizeof(type))
#define AllocArray(pool, count, type) (type*) AllocMemory(pool, (count) * sizeof(type))
void* AllocMemory(MemoryPool* pool, size_t size)
{
    Assert((pool->used + size <= pool->size));
    void* result = pool->base + pool->used;
    pool->used += size;
    return result;
}

void SetTileValue(MemoryPool* pool, TileMap* tileMap, u32 absTileX, u32 absTileY, u32 tileValue)
{
    TileChunkPosition chunkPos = GetChunkPosition(tileMap, absTileX, absTileY);
    TileChunk* tileChunk = GetTileChunk(tileMap, chunkPos.tileChunkX, chunkPos.tileChunkY);
    Assert((tileChunk));
    Assert((chunkPos.tileX < tileMap->chunkDims));
    Assert((chunkPos.tileY < tileMap->chunkDims));

    if(!tileChunk->tiles)
    {                
        // Allocate the Tiles
        for(u32 y = 0; y < tileMap->tileChunkCountY; y++)
        {
            for(u32 x = 0; x < tileMap->tileChunkCountX; x++)
            {
                tileMap->tileChunks[y * tileMap->tileChunkCountX + x].tiles =
                    AllocArray(pool, tileMap->chunkDims * tileMap->chunkDims, u32);
            }
        }
    }
    
    tileChunk->tiles[chunkPos.tileY * tileMap->chunkDims + chunkPos.tileX] = tileValue;
}

void InitializeMemoryPool(MemoryPool* pool, size_t size, u8* base)
{
    pool->size = size;
    pool->base = base;
    pool->used = 0;
}

extern "C" UPDATE_RENDER(UpdateRender)
{    
    i32 tileInPixels = 60; //Translation of units in pixels
    i32 unitsToPixels = 0; //Translation of units in pixels

    Assert((sizeof(GameState) <= memory->permanentSize));
    GameState* gameState = (GameState*)memory->permanentBlock;

    //INITIALIZATION
    if(!memory->isInitialized)
    {
        srand(static_cast<int>(time(0)));
        
        gameState->player.pos.absTileX = 1;
        gameState->player.pos.absTileY = 1;

        gameState->player.pos.tileRel = Vec2(0.0f, 0.0f);

        gameState->bgImage = DEBUGLoadImage(render, "test/test_bg.png");
        gameState->playerImage = DEBUGLoadImage(render, "test/test_player.png");
        
        InitializeMemoryPool(&gameState->memoryPool, memory->permanentSize - sizeof(GameState),
                             (u8*)memory->permanentBlock + sizeof(GameState));
        
        // Allocate the world
        gameState->world = AllocStruct(&gameState->memoryPool, World);
        World* world = gameState->world;
        
        // Allocate tilemap        
        world->tileMap = AllocStruct(&gameState->memoryPool, TileMap);
        TileMap* tileMap = world->tileMap;

        tileMap->chunkShift = 4;
        tileMap->chunkMask = (1 << tileMap->chunkShift) - 1;
        tileMap->chunkDims = (1 << tileMap->chunkShift);

        tileMap->tileInUnits = 1.8f;
        
        // NOTE: Do we need this???
        r32 offsetX = 0;
        r32 offsetY = (r32)render->screenH - tileInPixels;
    
        tileMap->tileChunkCountX = 128;
        tileMap->tileChunkCountY = 128;

        // Allocate the tileChunks
        tileMap->tileChunks = AllocArray(&gameState->memoryPool,
                                         tileMap->tileChunkCountX * tileMap->tileChunkCountY,
                                         TileChunk);

        u32 tilesPerWidth = 16;
        u32 tilesPerHeight = 9;

        u32 screenX = 0;
        u32 screenY = 0;
        // Set the tiles
        for(u32 screenIndex = 0;
            screenIndex < 10;
            screenIndex++)
        {
            u32 random = rand() % 1000;
            u32 randomNumber = random % 2;
            
            for(u32 tileY = 0; tileY < tilesPerHeight; tileY++)
            {
                for(u32 tileX = 0; tileX < tilesPerWidth; tileX++)
                {
                    u32 absTileX = screenX * tilesPerWidth + tileX;
                    u32 absTileY = screenY * tilesPerHeight + tileY;

                    u32 tileValue = 1;

                    //WALLS
                    if(tileX == 0 || tileX == tilesPerWidth - 1)
                    {
                        tileValue = 2;
                    }
                    if(tileY == 0 || tileY == tilesPerHeight - 1)
                    {
                        tileValue = 2;
                    }

                    //DOORS
                    if(tileX == (u32)(tilesPerWidth * 0.5f))
                    {
                        tileValue = 1;
                    }
                    if(tileY == (u32)(tilesPerHeight * 0.5f))
                    {
                        tileValue = 1;
                    }
                        
                    SetTileValue(&gameState->memoryPool, tileMap, absTileX, absTileY, tileValue);
                }
            }

            if(randomNumber == 0)
            {
                screenX++;
            }
            else if(randomNumber == 1)
            {
                screenY++;
            }
        }
        
        memory->isInitialized = true;
    }
    
    World* world = gameState->world;
    TileMap* tileMap = world->tileMap;

    unitsToPixels = (i32)(tileInPixels / tileMap->tileInUnits);
    
    r32 playerHeight = (r32)tileMap->tileInUnits;
    r32 playerWidth = (playerHeight * 0.75f);
    
    //UPDATE
    vec2 dir = {};
    
    bool isAnim = false;
    
    if(input->moveRight.isDown)
    {
        isAnim = true;
        gameState->facingDir = 3;
        dir.x = 1;
    }
    else if(input->moveLeft.isDown)
    {
        isAnim = true;
        gameState->facingDir = 2;
        dir.x = -1;
    }

    if(input->moveUp.isDown)
    {
        isAnim = true;
        gameState->facingDir = 1;
        dir.y = 1;
    }
    else if(input->moveDown.isDown)
    {
        isAnim = true;
        gameState->facingDir = 0;
        dir.y = -1;
    }

    r32 speed = 10.0f;
    if(input->actionUp.isDown)
    {
        speed = 50.0f;
    }

    dir *= speed;
    dir += -2.5f * gameState->oldVel;
    
    WorldPosition newPlayerPos = gameState->player.pos;
    newPlayerPos.tileRel = 0.5f * dir * Square(input->dt) + gameState->oldVel * input->dt + newPlayerPos.tileRel;
    gameState->oldVel = dir * input->dt + gameState->oldVel;
    
    newPlayerPos = RecalcWorldPos(tileMap, newPlayerPos);

    WorldPosition playerLeft = newPlayerPos;
    playerLeft.tileRel.x -= 0.5f * playerWidth;
    playerLeft = RecalcWorldPos(tileMap, playerLeft);
        
    WorldPosition playerRight = newPlayerPos;
    playerRight.tileRel.x += 0.5f * playerWidth;
    playerRight = RecalcWorldPos(tileMap, playerRight);
        
    if(IsTileMapLocEmpty(tileMap, newPlayerPos) &&
       IsTileMapLocEmpty(tileMap, playerLeft) &&
       IsTileMapLocEmpty(tileMap, playerRight))
    {
        gameState->player.pos = newPlayerPos;
    }
        
    // RENDER
    SDL_Rect viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.w = render->screenW;
    viewport.h = render->screenH;
    
    SDL_RenderSetViewport(render->renderer, &viewport);
    
    SetDrawColor(render, 0.5f, 0.75f, 1.0f);
    SDL_RenderClear(render->renderer);

    //BG IMAGE DRAW
    SDL_RenderCopy(render->renderer, gameState->bgImage, 0, &viewport);
    
    r32 screenCenterX = 0.5f * render->screenW;
    r32 screenCenterY = 0.5f * render->screenH;
    
    //DRAWING MAP
    for(i32 relRow = -10;
        relRow < 10; relRow++)
    {
        for(i32 relCol = -20;
            relCol < 20; relCol++)
        {
            i32 col = gameState->player.pos.absTileX + relCol;
            i32 row = gameState->player.pos.absTileY + relRow;

            u32 tileID = GetTileValue(tileMap, col, row);
                
            // NOTE:
            // id = 0: invalid
            // id = 1: walkable
            // id = 2: wall
            if(tileID > 0)
            {
                if(tileID == 2)
                {
                    r32 color = 1.0f;
                    
                    if((u32)col == gameState->player.pos.absTileX &&
                       (u32)row == gameState->player.pos.absTileY)
                    {
                        color = 0.0f;
                    }

                    r32 centerX = screenCenterX - unitsToPixels * gameState->player.pos.tileRel.x +
                        (r32)relCol * tileInPixels;
                    r32 centerY = screenCenterY + unitsToPixels * gameState->player.pos.tileRel.y -
                        (r32)relRow * tileInPixels;
                
                    r32 posX = centerX - 0.5f * (r32)tileInPixels;
                    r32 posY = centerY - 0.5f * (r32)tileInPixels;
                    r32 width = (r32)tileInPixels;
                    r32 height = (r32)tileInPixels;
                
                    SetDrawColor(render, color, color, color);
                    DrawRect(render, (i32)posX, (i32)(posY), (i32)width, (i32)height);
                }
            }
        }
    }

    //PLAYER DRAW
    r32 playerCenterX = screenCenterX - unitsToPixels * playerWidth * 0.5f;
    r32 playerCenterY = screenCenterY - unitsToPixels * playerHeight;
    
    SetDrawColor(render, .0f, 1.0f, 0.5f);
    DrawRect(render, (i32)playerCenterX, (i32)playerCenterY,
           (i32)(unitsToPixels * playerWidth), (i32)(unitsToPixels * playerHeight));

    if(isAnim)
    {
        i32 frameCount = 3;

        gameState->frameTimeElapsed += input->dt;
        
        if(gameState->frameTimeElapsed >= 0.1998f)
        {
            gameState->frame++;
            gameState->frame %= frameCount;

            gameState->frameTimeElapsed = 0;
        }
    }
    else
    {
        gameState->frameTimeElapsed = 0;
        gameState->frame = 1;
    }
    
    SDL_Rect src = {};
    src.x = gameState->frame * 16;
    src.y = (gameState->facingDir == 3 ? 2 : gameState->facingDir) * 16;
    src.w = 16;
    src.h = 16;

    SDL_Rect dest = {};
    dest.x = (i32)playerCenterX;
    dest.y = (i32)playerCenterY;
    dest.w = (i32)(unitsToPixels * playerWidth);
    dest.h = (i32)(unitsToPixels * playerHeight);

    SDL_RendererFlip facingRight = (gameState->facingDir == 3 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    SDL_RenderCopyEx(render->renderer, gameState->playerImage, &src, &dest, 0, 0, facingRight);
        
    SDL_RenderPresent(render->renderer);
}
