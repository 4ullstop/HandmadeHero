#if !defined(HANDMADE_H)

/*
  TODO: Services that the platform layer provides to the game
*/

/*
  Note: Serverices that the game provides to the platform layer
  (this may expandin the future - sound on separate thread, etc...)
*/

//Four things: timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

struct game_offscreen_buffer
{
    void* memory;
    int height;
    int width;
    int pitch;
    int bytesPerPixel;
};

struct game_sound_output_buffer
{
    int16* samples;
    int sampleCount;
    int samplesPerSecond;
};

internal void GameUpdateAndRender(game_offscreen_buffer* buffer, int blueOffset, int greenOffset, game_sound_output_buffer* soundBuffer, int toneHz);

#define HANDMADE_H
#endif



