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
	    uint8 blue = (x + xOffset);
	    uint8 green = (y + yOffset);
	    
	    *pixel++ = ((green << 8) | blue);

	}
	row += buffer->pitch;
    }
}

internal void
GameUpdateAndRender(game_input* input, game_offscreen_buffer* buffer, game_sound_output_buffer* soundBuffer)
{
    //TODO: Allow sample offsets here for more robust platform options
    local_persist int blueOffset = 0;
    local_persist int greenOffset = 0;
    local_persist int toneHz = 256;

    game_controller_input* input0 = &input->controllers[0];
    if (input0->isAnalog)
    {
	//use analog movement tuning

	    blueOffset += (int)4.0f*(input0->endX);
	    toneHz = 256 + (int)(128.0f*(input0->endY));
    }
    else
    {
	//use digital movement tuning
    }

    if (input0->down.endedDown)
    {
	greenOffset += 1;
    }
    GameOutputSound(soundBuffer, toneHz);
    RenderGradient(buffer, blueOffset, greenOffset);
}
