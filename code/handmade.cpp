#include "handmade.h"

internal void
GameOutputSound(game_sound_output_buffer *soundBuffer, int toneHz)
{
    local_persist real32 tSine;
    int16 toneVolume = 3000;
    int16* sampleOut = soundBuffer->samples;
    int wavePeriod = soundBuffer->samplesPerSecond / toneHz;
    for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex)
    {
	real32 sineValue = sinf(tSine);
	int16 sampleValue = (int16)(sineValue * toneVolume);
	*sampleOut++ = sampleValue;
	*sampleOut++ = sampleValue;
	
	tSine += 2.0f*Pi32*1.0f/(real32)wavePeriod;
    }
}

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
	    
	    *pixel++ = ((green << 8) | blue);

	}
	row += buffer->pitch;
    }
}

internal void
GameUpdateAndRender(game_memory* memory, game_input* input, game_offscreen_buffer* buffer, game_sound_output_buffer* soundBuffer)
{
    Assert((&input->controllers[0].terminator - &input->controllers[0].buttons[0]) ==
	   (ArrayCount(input->controllers[0].buttons)));
    Assert(sizeof(game_state) <= memory->permanentStorageSize);

    game_state* gameState = (game_state*)memory->permanentStorage;

    if (!memory->isInitialized)
    {
	char* fileName = __FILE__;

	debug_read_file_result file = DEBUGPlatformReadEntireFile(fileName);
	if (file.contents)
	{
	    DEBUGPlatformWriteEntireFile("w:/data/test.out", file.contentsSize, file.contents);
	    DEBUGPlatformFreeFileMemory(file.contents);
	}
			
	gameState->toneHz = 256;
	memory->isInitialized = true;
    }

    for (int controllerIndex = 0; controllerIndex < ArrayCount(input->controllers); ++controllerIndex)
    {
	game_controller_input* controller = GetController(input, controllerIndex);
	if (controller->isAnalog)
	{
	    //use analog movement tuning

	    gameState->blueOffset += (int)(4.0f*(controller->stickAverageX));
	    gameState->toneHz = 256 + (int)(128.0f*(controller->stickAverageY));
	}
	else
	{
	    //use digital movement tuning
	    if (controller->moveLeft.endedDown)
	    {
		gameState->blueOffset -= 1;
	    }
	    
	    if (controller->moveRight.endedDown)
	    {
		gameState->blueOffset += 1;
	    }
	}

	if (controller->actionDown.endedDown)
	{
	    gameState->greenOffset += 1;
	}
    }
    
    GameOutputSound(soundBuffer, gameState->toneHz);
    RenderGradient(buffer, gameState->blueOffset, gameState->greenOffset);
}
