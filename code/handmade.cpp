#include "handmade.h"

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

internal int32
RoundReal32ToInt32(real32 real32)
{
    int32 result = (int32)(real32 + 0.5f);
    return(result);
}

internal uint32
RoundReal32ToUInt32(real32 real32)
{
    uint32 result = (uint32)(real32 + 0.5f);
    return(result);
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

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
	   (ArrayCount(input->controllers[0].buttons)));
    Assert(sizeof(game_state) <= memory->permanentStorageSize);

    game_state* gameState = (game_state*)memory->permanentStorage;

    if (!memory->isInitialized)
    {
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

	    dPlayerX *= 128.0f;
	    dPlayerY *= 128.0f;
	    
	    gameState->playerX += input->dTime * dPlayerX;
	    gameState->playerY += input->dTime * dPlayerY;
	}
    }

    uint32 tileMap[9][17] =
	{
	    {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 1, 1, 1,  1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},
	    {0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1},	    
	};
    real32 upperLeftX = -30;
    real32 upperLeftY = 0;
    real32 tileWidth = 60;
    real32 tileHeight = 60;
    
    DrawRectangle(buffer, 0.0f, 0.0f, (real32)buffer->width, (real32)buffer->height, 1.0f, 0.0f, 1.0f);
    for (int row = 0; row < 9; ++row)
    {
	for (int column = 0; column < 17; ++column)
	{
	    uint32 tileId = tileMap[row][column];
	    real32 gray = 0.5f;
	    if (tileId == 1)
	    {
		gray = 1.0f;
	    }

	    real32 minX = upperLeftX + ((real32)column) * tileWidth;
	    real32 minY = upperLeftY + ((real32)row) * tileHeight;
	    real32 maxX = minX + tileWidth;
	    real32 maxY = minY + tileHeight;
	    
	    DrawRectangle(buffer, minX, minY, maxX, maxY, gray, gray, gray);
	}
    }

 


    real32 playerR = 1.0f;
    real32 playerG = 1.0f;
    real32 playerB = 0.0f;

    real32 playerWidth = 0.75f*(real32)tileWidth;
    real32 playerHeight = (real32)tileHeight;

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
