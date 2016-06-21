#include "Game.h"

static i32 xPos = 0;
static i32 yPos = 0;

inline UPDATE_RENDER(UpdateRender)
{
	//UPDATE
	SDL_Rect rect = {};
	rect.x = xPos++;
	rect.y = yPos;
	rect.w = 100;
	rect.h = 100;

	if(xPos > 1024)
	{
		xPos = 0;
	}

	//RENDER
	SDL_SetRenderDrawColor(renderer, 50, 150, 250, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(renderer, &rect);

	SDL_RenderPresent(renderer);
}