#include "Game.h"

/*
	2D Top Down game ITS FINAL!!
	walking 4 ways (up down left right).
*/

struct Entity
{
	r32 xPos;
	r32 yPos;
};

static Entity* player = 0;

inline UPDATE_RENDER(UpdateRender)
{
	if(!gameState->isInitialized)
	{
		player = (Entity*)gameState->memoryBlock;
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

		//Player movement
		player->xPos += xDir * 50.0f * gameState->dt;
		player->yPos += yDir * 50.0f * gameState->dt;

		SDL_Rect rect = {0};
		rect.x = player->xPos;
		rect.y = player->yPos;
		rect.w = 100 * 0.75f;
		rect.h = 100;

		if(player->xPos > gameState->screenW)
		{
			player->xPos = 0;
		}

		SDL_SetRenderDrawColor(gameState->renderer, 50, 150, 250, 255);
		SDL_RenderClear(gameState->renderer);

		SDL_SetRenderDrawColor(gameState->renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(gameState->renderer, &rect);

		SDL_RenderPresent(gameState->renderer);
	}
}