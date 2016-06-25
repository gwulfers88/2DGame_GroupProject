#include "Game.h"

/*
    2D Top Down game ITS FINAL!!
    walking 4 ways (up down left right).
*/

// IMPORTANT: Moved Entity struct to Game.h

void AddPlayer(GameState* gameState, GameMemory* memory)
{
    gameState->player = (Entity*)memory->memoryBlock;
    gameState->player->type = ENTITY_PLAYER;
    gameState->player->xPos = 0;
    gameState->player->yPos = 0;
    gameState->entityCount++;       
}

extern "C" UPDATE_RENDER(UpdateRender)
{
    if(!gameState->isInitialized)
    {
        AddPlayer(gameState, memory);
        
        gameState->isInitialized = true;
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

        r32 speed = 100.0f;

        gameState->player;

        gameState->player->xPos += xDir * speed * gameState->dt;
        gameState->player->yPos += yDir * speed * gameState->dt;

        // RENDER
        SDL_SetRenderDrawColor(gameState->renderer, 50, 150, 250, 255);
        SDL_RenderClear(gameState->renderer);

        SDL_Rect rect = {0};
        rect.x = (i32)gameState->player->xPos;
        rect.y = (i32)gameState->player->yPos;
        rect.w = (i32)(100 * 0.75f);
        rect.h = 100;

        SDL_SetRenderDrawColor(gameState->renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(gameState->renderer, &rect);

        SDL_RenderPresent(gameState->renderer);
    }
}
