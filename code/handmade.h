#if !defined(HANDMADE_H)

/*
  TODO: Services that the platform layer provides to the game
*/


#if HANDMADE_INTERNAL
struct debug_read_file_result
{
    uint32 contentsSize;
    void* contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char* fileName);
internal void DEBUGPlatformFreeFileMemory(void* memory);

internal bool32 DEBUGPlatformWriteEntireFile(char* fileName, uint32 memorySize, void* memory);
#endif

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

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0])) 

inline uint32
SafeTruncateUInt64(uint64 value)
{
    Assert(value <= 0xFFFFFFFF);
    uint32 result = (uint32)value;
    return(result);
}

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
    bool32 isConnected;

    real32 stickAverageX;
    real32 stickAverageY;
    
    union
    {
	game_button_state buttons[10];
	struct
	{
	    game_button_state moveUp;
	    game_button_state moveDown;
	    game_button_state moveLeft;
	    game_button_state moveRight;
	    
	    game_button_state actionUp;
	    game_button_state actionDown;
	    game_button_state actionLeft;
	    game_button_state actionRight;
	    
	    game_button_state leftShoulder;
	    game_button_state rightShoulder;

	    game_button_state back;
	    game_button_state start;
	};
    };
};

struct game_input
{
    game_controller_input controllers[5];
};
inline game_controller_input *GetController(game_input* input, int controllerIndex)
{
    Assert(controllerIndex < ArrayCount(input->controllers));
    game_controller_input* result = &input->controllers[controllerIndex]; 
    return(result);
}


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


