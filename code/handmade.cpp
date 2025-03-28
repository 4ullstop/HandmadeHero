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

internal void
DrawRectangle(game_offscreen_buffer* backBuffer,
	      real32 realMinX, real32 realMinY,
	      real32 realMaxX, real32 realMaxY)
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
    
    
    uint32 color = 0xFFFFFFFF;

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
	    
	}
    }

    DrawRectangle(buffer, 10.0f, 10.0f, 30.0f, 30.0f);
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
