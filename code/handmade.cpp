#include "handmade.h"

internal void
GameOutputSound(game_state* gameState, game_sound_output_buffer *soundBuffer, int toneHz)
{
    int16 toneVolume = 3000;
    int16* sampleOut = soundBuffer->samples;
    int wavePeriod = soundBuffer->samplesPerSecond / toneHz;
    for (int sampleIndex = 0; sampleIndex < soundBuffer->sampleCount; ++sampleIndex)
    {

	real32 sineValue = sinf(gameState->tSine);
	int16 sampleValue = (int16)(sineValue * toneVolume);



	*sampleOut++ = sampleValue;
	*sampleOut++ = sampleValue;
	gameState->tSine += 2.0f*Pi32*1.0f/(real32)wavePeriod;
	if (gameState->tSine > 2.0f*Pi32)
	{
	    gameState->tSine -= 2.0f*Pi32;
	}
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
	    
	    *pixel++ = ((green << 16) | blue);

	}
	row += buffer->pitch;
    }
}

internal void
RenderPlayer(game_offscreen_buffer* backBuffer, int playerX, int playerY)
{
    uint8* endOfBuffer = (uint8*)backBuffer->memory +
	backBuffer->pitch*backBuffer->height;
    uint32 color = 0xFFFFFFFF;
    int top = playerY;
    int bottom = playerY + 10;
    
    for (int x = playerX; x < playerX + 10; ++x)
    {
	uint8* pixel = ((uint8*)backBuffer->memory + x * backBuffer->bytesPerPixel + top * backBuffer->pitch);
	for (int y = top; y < bottom; ++y)
	{
	    if ((pixel >= backBuffer->memory) && (pixel < endOfBuffer))
	    {
		*(uint32*)pixel = color;
	    }
	    pixel += backBuffer->pitch;
	}
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
	char* fileName = __FILE__;

	debug_read_file_result file = memory->DEBUGPlatformReadEntireFile(thread, fileName);
	if (file.contents)
	{
	    memory->DEBUGPlatformWriteEntireFile(thread, "w:/data/test.out", file.contentsSize, file.contents);
	    memory->DEBUGPlatformFreeFileMemory(thread, file.contents);
	}
			
	gameState->toneHz = 256;
	gameState->tSine = 0.0f;
	memory->isInitialized = true;

	gameState->playerX = 100;
	gameState->playerY = 100;
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
	gameState->playerX += (int)(4.0f*controller->stickAverageX);
	gameState->playerY -= (int)(4.0f*controller->stickAverageY + 10.0f * sinf(gameState->tJump));
/*
	if (controller->actionDown.endedDown)
	{
	    gameState->tJump = 1.0;
	}
	gameState->tJump -= 0.033f;
*/	    
    }
    

    RenderGradient(buffer, gameState->blueOffset, gameState->greenOffset);
    RenderPlayer(buffer, gameState->playerX, gameState->playerY);
    RenderPlayer(buffer, input->mouseX, input->mouseY);
    for (int buttonIndex = 0; buttonIndex < ArrayCount(input->mouseButtons); ++buttonIndex)
    {
	if (input->mouseButtons[buttonIndex].endedDown)
	{
	    RenderPlayer(buffer, 10 + 20 * buttonIndex, 10);
	}
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state* gameState = (game_state*)memory->permanentStorage;
    GameOutputSound(gameState, soundBuffer, gameState->toneHz);
}
