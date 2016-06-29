#include "Game.h"

/*
    2D Top Down game ITS FINAL!!
    walking 4 ways (up down left right).
*/

// IMPORTANT: Moved Entity struct to Game.h

/*
  TODO(George): Things to do for monday!!!
  - Make multiple tile maps.
  - Be able to have the player go between maps (screens).
  - Add smooth scrolling.
  - Make world structure.
  - Calculating World positions from screen coords or pixel positions.
  - Make v2 math struct.
  - Make it possible to have multiple players in our game. (maybe 2 or 4 max).
*/

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

u32 GetTileID(TileMap* tileMap, u32 testTileX, u32 testTileY)
{
    u32 tileID = 0;
    tileID = tileMap->tiles[testTileY * tileMap->countX + testTileX];
    return tileID;
}

bool IsTileEmpty(TileMap* tileMap, i32 testX, i32 testY)
{
    bool empty = false;
    
    //Find out if the player position is inside of a tile that has a wall on it.
    u32 testTileX = (u32)((testX - (r32)tileMap->offsetX) / (r32)tileMap->width);
    u32 testTileY = (u32)((testY - (r32)tileMap->offsetY) / (r32)tileMap->height);

    empty = (GetTileID(tileMap, testTileX, testTileY) == 0);

    return empty;
}

extern "C" UPDATE_RENDER(UpdateRender)
{
    #define TILE_COUNT_X 13
    #define TILE_COUNT_Y 9
    u32 TileMaps[TILE_COUNT_Y][TILE_COUNT_X] =
        {
            {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1},
            {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
            {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1},
        };

    TileMap tileMap = {};

    tileMap.countX = TILE_COUNT_X;
    tileMap.countY = TILE_COUNT_Y;
    tileMap.width = 60;
    tileMap.height = 60;
    tileMap.offsetX = 30;
    tileMap.offsetY = 30;
    
    tileMap.tiles = (u32 *)TileMaps;

    i32 playerWidth = (i32)(tileMap.width * 0.75f);
    i32 playerHeight = tileMap.height;

    Assert((sizeof(GameState) <= memory->permanentSize));
    
    //TODO: Aseert that gameState size does not exceed memory size.
    GameState* gameState = (GameState*)memory->permanentBlock;

    //INITIALIZATION
    if(!memory->isInitialized)
    {
        gameState->player.xPos = 200;
        gameState->player.yPos = 200;
        
        memory->isInitialized = true;
    }
    else
    {
        //UPDATE
        r32 xDir = 0;
        r32 yDir = 0;

        if(input->actionRight.isDown)
        {
            xDir = 1;
        }
        else if(input->actionLeft.isDown)
        {
            xDir = -1;
        }

        if(input->actionUp.isDown)
        {
            yDir = -1;
        }
        else if(input->actionDown.isDown)
        {
            yDir = 1;
        }

        r32 speed = 128.0f;

        r32 newPlayerX = gameState->player.xPos;
        r32 newPlayerY = gameState->player.yPos;
        
        newPlayerX += xDir * speed * input->dt;
        newPlayerY += yDir * speed * input->dt;

        if(IsTileEmpty(&tileMap, (i32)newPlayerX - (i32)(playerWidth * 0.5f), (i32)newPlayerY) &&
           IsTileEmpty(&tileMap, (i32)newPlayerX + (i32)(playerWidth * 0.5f), (i32)newPlayerY) &&
           IsTileEmpty(&tileMap, (i32)newPlayerX, (i32)newPlayerY))
        {
            gameState->player.xPos = newPlayerX;
            gameState->player.yPos = newPlayerY;
        }
        
        // RENDER
        SetDrawColor(render, 0.5f, 0.75f, 1.0f);
        SDL_RenderClear(render->renderer);

        //DRAWING MAP
        for(i32 row = 0; row < (i32)tileMap.countY; row++)
        {
            for(i32 col = 0; col < (i32)tileMap.countX; col++)
            {
                u32 tileID = GetTileID(&tileMap, col, row);
                r32 color = 0.0f;
                
                if(tileID == 1)
                {
                    color = 1.0f;
        
                    u32 posX = tileMap.offsetX + col * tileMap.width;
                    u32 posY = tileMap.offsetY + row * tileMap.height;

                    SetDrawColor(render, color, color, color);
                    DrawRect(render, posX, posY, tileMap.width, tileMap.height);
                }
            }
        }
        
        r32 playerCenterX = gameState->player.xPos - (playerWidth * 0.5f);
        r32 playerCenterY = gameState->player.yPos - playerHeight;

        SetDrawColor(render, .0f, 1.0f, 0.5f);
        DrawRect(render, (i32)playerCenterX, (i32)playerCenterY, playerWidth, playerHeight);
        
        SDL_RenderPresent(render->renderer);
    }
}
