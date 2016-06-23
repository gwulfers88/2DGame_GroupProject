#include "Game.h"

/*
    2D Top Down game ITS FINAL!!
    walking 4 ways (up down left right).
*/

// NOTE: Moved Entity struct to Game.h

inline void movePlayer(GameState* gameState, Entity* entity, r32 xDir, r32 yDir)
{
    //Player movement
    entity->xPos += xDir * 50 * gameState->dt;
    entity->yPos += yDir * 50 * gameState->dt;
}

extern "C" UPDATE_RENDER(UpdateRender)
{
    if(!gameState->isInitialized)
    {
        gameState->player = (Entity*)gameState->memoryBlock;
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

        movePlayer(gameState, gameState->player, xDir, yDir);
        
        SDL_Rect rect = {0};
        rect.x = (u32)gameState->player->xPos;
        rect.y = (u32)gameState->player->yPos;
        rect.w = (u32)(100 * 0.75f);
        rect.h = 100;

        if(gameState->player->xPos > gameState->screenW)
        {
            gameState->player->xPos = 0;
        }
        else if(gameState->player->xPos > gameState->screenH)
        {
            gameState->player->yPos = 0;
        }

        SDL_SetRenderDrawColor(gameState->renderer, 50, 150, 250, 255);
        SDL_RenderClear(gameState->renderer);

        SDL_SetRenderDrawColor(gameState->renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(gameState->renderer, &rect);

        SDL_RenderPresent(gameState->renderer);
    }
}
