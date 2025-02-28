#include "handmade.h"


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
GameUpdateAndRender(game_offscreen_buffer* buffer, int blueOffset, int greenOffset)
{
    RenderGradient(buffer, blueOffset, greenOffset);
}
