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

internal canonical_position
GetCanonicalPosition(world* world, raw_position pos)
{
    canonical_position result;

    result.tileMapX = pos.tileMapX;
    result.tileMapY = pos.tileMapY;
    
    real32 x = pos.x - world->upperLeftX;
    real32 y = pos.y - world->upperLeftY;
    
    result.tileX = FloorReal32ToInt32(x / world->tileSideInPixels);
    result.tileY = FloorReal32ToInt32(y / world->tileSideInPixels);

    result.x = x - result.tileX * world->tileSideInPixels;
    result.y = y - result.tileY * world->tileSideInPixels;

    Assert(result.x >= 0);
    Assert(result.y >= 0);
    Assert(result.x < world->tileSideInPixels);
    Assert(result.y < world->tileSideInPixels);
    

    //If we step out of bounds, go to the next tile
    if (result.tileX < 0)
    {
	result.tileX = world->countX + result.tileX;
	--result.tileMapX;
    }

    if (result.tileY < 0)
    {
	result.tileY = world->countY + result.tileY;
	--result.tileMapY;
    }

    if (result.tileX >= world->countX)
    {
	result.tileX = result.tileX - world->countX;
	++result.tileMapX;
    }

    if (result.tileY >= world->countY)
    {
	result.tileY = result.tileY - world->countY;
	++result.tileMapY;
    }
    return(result);
}

internal bool32
IsWorldPointEmpty(world* world, raw_position testPos)
{
    bool32 empty = false;

    canonical_position canPos = GetCanonicalPosition(world, testPos);
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

    world.upperLeftY = 0;
    world.upperLeftX = -(real32)world.tileSideInPixels / 2;

    real32 playerWidth = 0.75f*(real32)world.tileSideInPixels;
    real32 playerHeight = (real32)world.tileSideInPixels;
    
    tileMaps[0][0].tiles = (uint32*)tiles00;
    tileMaps[0][1].tiles = (uint32*)tiles10;
    tileMaps[1][0].tiles = (uint32*)tiles01;
    tileMaps[1][1].tiles = (uint32*)tiles11;
    
    game_state* gameState = (game_state*)memory->permanentStorage;

    tile_map* tileMap = GetTileMap(&world, gameState->playerTileMapX, gameState->playerTileMapY);
    Assert(tileMap);

				   


    if (!memory->isInitialized)
    {
	gameState->playerX = 150;
	gameState->playerY = 150;
	
	memory->isInitialized = true;
    }

        
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

	    dPlayerX *= 64.0f;
	    dPlayerY *= 64.0f;

	    real32 newPlayerX = gameState->playerX + input->dTime * dPlayerX;
	    real32 newPlayerY = gameState->playerY + input->dTime * dPlayerY;

	    raw_position playerPos =
		{gameState->playerTileMapX, gameState->playerTileMapY,
		newPlayerX, newPlayerY};

	    raw_position playerLeft = playerPos;
	    playerLeft.x -= 0.5f * playerWidth;
	    raw_position playerRight = playerPos;
	    playerRight.x += 0.5f * playerWidth;
	    
	    if (IsWorldPointEmpty(&world, playerPos) &&
		IsWorldPointEmpty(&world, playerLeft) &&
		IsWorldPointEmpty(&world, playerRight))
	    {
		canonical_position canPos = GetCanonicalPosition(&world, playerPos);
		gameState->playerTileMapX = canPos.tileMapX;
		gameState->playerTileMapY = canPos.tileMapY;
	        gameState->playerX = world.upperLeftX + world.tileSideInPixels * canPos.tileX + canPos.x;
		gameState->playerY = world.upperLeftY + world.tileSideInPixels * canPos.tileY +  canPos.y;
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

    real32 playerTop = gameState->playerY - playerHeight;
    real32 playerLeft = gameState->playerX - 0.5f * playerWidth;
    
    DrawRectangle(buffer, playerLeft, playerTop,
		  playerLeft + playerWidth,
		  playerTop + playerHeight,
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
 
