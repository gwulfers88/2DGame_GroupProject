#if !defined(GAME_TILES_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: George Wulfers $
   $Notice: (C) Copyright 2016 by 8Box Studios, LLC. All Rights Reserved. $
   ======================================================================== */
#include "vec2.h"

struct TileChunkPosition
{
    u32 tileChunkX;
    u32 tileChunkY;

    u32 tileX;
    u32 tileY;
};

struct WorldPosition
{
    u32 absTileX;
    u32 absTileY;

    vec2 tileRel;
};

struct TileChunk
{
    u32 *tiles;
};

struct TileMap
{
    u32 chunkShift;
    u32 chunkMask;
    u32 chunkDims;
    
    r32 tileInUnits;  //translation of units

    u32 tileChunkCountX;
    u32 tileChunkCountY;
    
    TileChunk* tileChunks;
};

#define GAME_TILES_H
#endif
