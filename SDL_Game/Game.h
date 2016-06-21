#ifndef GAME_H
#define GAME_H

#include "defs.h"

#define UPDATE_RENDER(name) void name(SDL_Renderer* renderer)
typedef UPDATE_RENDER(update_render);

#endif