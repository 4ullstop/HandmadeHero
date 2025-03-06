#if !defined(HANDMADE_H)

/*
  TODO: Services that the platform layer provides to the game
*/

/*
  Note: Serverices that the game provides to the platform layer
  (this may expandin the future - sound on separate thread, etc...)
*/

//Four things: timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

/*
  HANDMADE_INTERNAL:
   0 - Build for public release
   1 - Build for developer only

   HANDMADE_SLOW
    0 - No slow code allowed!
    1 - Slow code welcome
 */

#if HANDMADE_SLOW
#define Assert(Expression) if (!(Expression)) *(int *)0 = 0;
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

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

struct game_memory
{
    bool32 isInitialized;
    
    uint64 permanentStorageSize;
    void* permanentStorage;

    uint64 transientStorageSize;
    void* transientStorage;
};

internal void GameUpdateAndRender(game_memory* memory, game_input* input, game_offscreen_buffer* buffer, game_sound_output_buffer* soundBuffer);


//
//
//

struct game_state
{
    int toneHz;
    int greenOffset;
    int blueOffset;
};

#define HANDMADE_H
#endif



