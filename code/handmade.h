#if !defined(HANDMADE_H)

/*
  TODO: Services that the platform layer provides to the game
*/

/*
  Note: Serverices that the game provides to the platform layer
  (this may expandin the future - sound on separate thread, etc...)
*/

//Four things: timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0])) 

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

struct game_button_state
{
    int halfTransitionCount;
    bool32 endedDown;
};

struct game_controller_input
{
    bool32 isAnalog; 
    
    real32 startX;
    real32 startY;

    real32 minX;
    real32 minY;

    real32 maxX;
    real32 maxY;
    
    real32 endX;
    real32 endY;
    union
    {
	struct
	{
	    
	    game_button_state up;
	    game_button_state down;
	    game_button_state left;
	    game_button_state right;
	    game_button_state leftShoulder;
	    game_button_state rightShoulder;
	};
    };
};

struct game_input
{
    game_controller_input controllers[4];
};

internal void GameUpdateAndRender(game_input* input, game_offscreen_buffer* buffer, game_sound_output_buffer* soundBuffer);

#define HANDMADE_H
#endif



