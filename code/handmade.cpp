#include "handmade.h"
#include "handmade_intrinsics.h"

internal void
GameOutputSound(game_state* gameState, game_sound_output_buffer *soundBuffer, int toneHz)
{
    int16 toneVolume = 3000;
    int16* sampleOut = soundBuffer->samples;
    int wavePeriod = soundBuffer->samplesPerSecond / toneHz;
    for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex)
    {

#if 0 	
	real32 sineValue = sinf(gameState->tSine);
	int16 sampleValue = (int16)(sineValue * toneVolume);
#else
	int16 sampleValue = 0;
#endif	


	*sampleOut++ = sampleValue;
	*sampleOut++ = sampleValue;
#if 0
	gameState->tSine += 2.0f*Pi32*1.0f/(real32)wavePeriod;
	if (gameState->tSine > 2.0f*Pi32)
	{
	    gameState->tSine -= 2.0f*Pi32;
	}
#endif	
    }
}

internal void
DrawRectangle(game_offscreen_buffer* backBuffer,
	      real32 realMinX, real32 realMinY,
	      real32 realMaxX, real32 realMaxY,
	      real32 r, real32 g, real32 b)
{
    //rect gets filled up to but not including the final row
    int32 minX = RoundReal32ToInt32(realMinX);
    int32 minY = RoundReal32ToInt32(realMinY);
    int32 maxX = RoundReal32ToInt32(realMaxX);
    int32 maxY = RoundReal32ToInt32(realMaxY);

    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;

    if (maxX > backBuffer->width) maxX = backBuffer->width;
    if (maxY > backBuffer->height) maxY = backBuffer->height;
    
    //Bit patter: 0x AA RR GG BB
    //8 bit values
    //255 at max and 0 at min
    uint32 color = ((RoundReal32ToUInt32(r * 255.0f) << 16) | 
		    (RoundReal32ToUInt32(g * 255.0f) << 8) |
		    (RoundReal32ToUInt32(b * 255.0f) << 0));

    //points to the top left corner of the rectangle
    uint8* row = ((uint8*)backBuffer->memory + minX * backBuffer->bytesPerPixel + minY * backBuffer->pitch);
    
    for (int y = minY; y < maxY; ++y)
    {
	uint32* pixel = (uint32*)row;
	for (int x = minX; x < maxX; ++x)
	{
	    *(uint32*)pixel++ = color;
	}
	//pitch is in bytes so it's 8 bytes
	row += backBuffer->pitch;
    }
}

inline tile_map* 
GetTileMap(world* world, int32 tileMapX, int32 tileMapY)
{
    tile_map* tileMap = 0;

    if ((tileMapX >= 0) && (tileMapX < world->tileMapCountX) &&
	(tileMapY >= 0) && (tileMapY < world->tileMapCountY))
    {
	tileMap = &world->tileMaps[tileMapY * world->tileMapCountX + tileMapX];
    }

    return(tileMap);
}

inline uint32
GetTileMapValueUnchecked(world* world, tile_map* tileMap, int32 tileX, int32 tileY)
{
    Assert(tileMap);
    Assert((tileX >= 0) && (tileX < world->countX) &&
	   (tileY >= 0) && (tileY < world->countY));
    uint32 result = tileMap->tiles[tileY*world->countX + tileX];
    return(result);
}

internal bool32
IsTileMapPointEmpty(world* world, tile_map* tileMap, int32 testTileX, int32 testTileY)
{
    bool32 empty = false;
    
    if (tileMap)
    {
	if ((testTileX >= 0) && (testTileX < world->countX) && 
	    (testTileY >= 0) && (testTileY < world->countY))
	{
	    uint32 tileMapValue = GetTileMapValueUnchecked(world, tileMap, testTileX, testTileY);
	    empty = (tileMapValue == 0);
	}
    }
    return(empty);
}

inline void
RecanonicalizeCoord(world* world, int32 tileCount, int32* tileMap, int32* tile, real32* tileRel)
{
    int32 offset = FloorReal32ToInt32(*tileRel / world->tileSideInMeters);
    *tile += offset;
    *tileRel -= offset*world->tileSideInMeters;

    Assert(*tileRel >= 0);
    Assert(*tileRel < world->tileSideInMeters);

     

    //If we step out of bounds, go to the next tile
    if (*tile < 0)
    {
	*tile = tileCount + *tile;
	--*tileMap;
    }

    if (*tile >= tileCount)
    {
	*tile = *tile - tileCount;
	++*tileMap;
    }

}

inline canonical_position
RecanonicalizePosition(world* world, canonical_position pos)
{
    canonical_position result = pos;

    //between 0 and 1, then we haven't gone outside
    //less than 0, you have gone too far left
    //greater than 1 and you have gone too far right
    RecanonicalizeCoord(world, world->countX, &result.tileMapX, &result.tileX, &result.x);
    RecanonicalizeCoord(world, world->countY, &result.tileMapY, &result.tileY, &result.y);
    
    return(result);
}

internal bool32
IsWorldPointEmpty(world* world, canonical_position canPos)
{
    bool32 empty = false;

    tile_map* tileMap = GetTileMap(world, canPos.tileMapX, canPos.tileMapY);
    empty = IsTileMapPointEmpty(world, tileMap, canPos.tileX, canPos.tileY);
    
    return(empty);    
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
	   (ArrayCount(input->controllers[0].buttons)));
    Assert(sizeof(game_state) <= memory->permanentStorageSize);
    
#define TILE_MAP_COUNT_X 17
#define TILE_MAP_COUNT_Y 9
    uint32 tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
	{1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0},
	{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1}    
    };
    
    uint32 tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1}
    };

    uint32 tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1}	    
    };

    uint32 tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
 	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1}	    
    };
    
    
    tile_map tileMaps[2][2];

    
    world world;
    world.tileMaps = (tile_map*)tileMaps;
    world.tileMapCountX = 2;
    world.tileMapCountY = 2;    
    world.countX = TILE_MAP_COUNT_X;
    world.countY = TILE_MAP_COUNT_Y;

    world.tileSideInMeters = 1.4f;
    world.tileSideInPixels = 60;
    world.metersToPixels = (real32)world.tileSideInPixels / (real32)world.tileSideInMeters;

    
    world.upperLeftY = 0;
    world.upperLeftX = -(real32)world.tileSideInPixels / 2;

    real32 playerHeight = 1.4f;
    real32 playerWidth = 0.75f*playerHeight;

    
    tileMaps[0][0].tiles = (uint32*)tiles00;
    tileMaps[0][1].tiles = (uint32*)tiles10;
    tileMaps[1][0].tiles = (uint32*)tiles01;
    tileMaps[1][1].tiles = (uint32*)tiles11;
    
    game_state* gameState = (game_state*)memory->permanentStorage;


    if (!memory->isInitialized)
    {
	gameState->playerP.tileMapX = 0;
	gameState->playerP.tileMapY = 0;
	gameState->playerP.tileX = 3;
	gameState->playerP.tileY = 3;
	gameState->playerP.x = 5.0f;
	gameState->playerP.y = 5.0f;

	
	memory->isInitialized = true;
    }

    
    tile_map* tileMap = GetTileMap(&world, gameState->playerP.tileMapX, gameState->playerP.tileMapY);
    Assert(tileMap); 
        
    for (int controllerIndex = 0; controllerIndex < ArrayCount(input->controllers); ++controllerIndex)
    {
	game_controller_input* controller = GetController(input, controllerIndex);
	if (controller->isAnalog)
	{
 	    //use analog movement tuning
	}
	else
	{
	    real32 dPlayerX = 0.0f; //pixels per second
	    real32 dPlayerY = 0.0f;

	    if (controller->moveUp.endedDown)
	    {
		dPlayerY = -1.0f;
	    }
	    if (controller->moveLeft.endedDown)
	    {
		dPlayerX = -1.0f;
	    }
	    if (controller->moveRight.endedDown)
	    {
		dPlayerX = 1.0f;
	    }
	    if (controller->moveDown.endedDown)
	    {
		dPlayerY = 1.0f;
	    }

	    dPlayerX *= 2.0f;
	    dPlayerY *= 2.0f;

	    canonical_position newPlayerP = gameState->playerP;
	    newPlayerP.x += input->dTime * dPlayerX;
	    newPlayerP.y += input->dTime * dPlayerY;
	    newPlayerP = RecanonicalizePosition(&world, newPlayerP);
	    
	    canonical_position playerLeft = newPlayerP;
	    playerLeft.x -= 0.5f*playerWidth;
	    playerLeft = RecanonicalizePosition(&world, playerLeft);
	    
	    canonical_position playerRight = newPlayerP;
	    playerRight.x += 0.5f*playerWidth;
	    playerRight = RecanonicalizePosition(&world, playerRight);
	    
	    if (IsWorldPointEmpty(&world, newPlayerP) &&
		IsWorldPointEmpty(&world, playerLeft) &&
		IsWorldPointEmpty(&world, playerRight))
	    {
		gameState->playerP = newPlayerP;
	    }
	}
    }

    DrawRectangle(buffer, 0.0f, 0.0f, (real32)buffer->width, (real32)buffer->height, 1.0f, 0.0f, 1.0f);
    for (int row = 0; row < 9; ++row)
    {
	for (int column = 0; column < 17; ++column)
	{
	    uint32 tileId = GetTileMapValueUnchecked(&world, tileMap, column, row);
	    real32 gray = 0.5f;
	    if (tileId == 1)
	    {
		gray = 1.0f;
	    }

	    if ((column == gameState->playerP.tileX) && (row == gameState->playerP.tileY))
	    {
		gray = 0.0f;
	    }
	    
	    real32 minX = world.upperLeftX + ((real32)column) * world.tileSideInPixels;
	    real32 minY = world.upperLeftY + ((real32)row) * world.tileSideInPixels;
	    real32 maxX = minX + world.tileSideInPixels;
	    real32 maxY = minY + world.tileSideInPixels;
	    DrawRectangle(buffer, minX, minY, maxX, maxY, gray, gray, gray);
	}
    }

 


    real32 playerR = 1.0f;
    real32 playerG = 1.0f;
    real32 playerB = 0.0f;
    real32 playerLeft = world.upperLeftX + world.tileSideInPixels * gameState->playerP.tileX
	+ world.metersToPixels * gameState->playerP.x - 0.5f * world.metersToPixels * playerWidth;
    real32 playerTop = world.upperLeftY + world.tileSideInPixels * gameState->playerP.tileY
	+ world.metersToPixels * gameState->playerP.y - world.metersToPixels * playerHeight;

    
    DrawRectangle(buffer, playerLeft, playerTop,
		  playerLeft + playerWidth * world.metersToPixels,
		  playerTop + playerHeight * world.metersToPixels,
		  playerR, playerG, playerB);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state* gameState = (game_state*)memory->permanentStorage;
    GameOutputSound(gameState, soundBuffer, 400);
}

/*
internal void
RenderGradient(game_offscreen_buffer* buffer, int xOffset, int yOffset)
{
    uint8* row = (uint8*)buffer->memory;
    for (int y = 0; y < buffer->height; ++y)
    {
	uint32* pixel = (uint32*)row;
	for (int x = 0; x < buffer->width; ++x)
	{
	    uint8 blue = (uint8)(x + xOffset);
	    uint8 green = (uint8)(y + yOffset);
	    
	    *pixel++ = ((green << 16) | blue);

	}
	row += buffer->pitch;
    }
}
*/
 
