#if !defined(HANDMADE_H)

/*
  TODO: Services that the platform layer provides to the game
*/

#include <math.h>
#include <stdint.h>

#define local_persist static  //locally created variable - exist after creation
#define global_variable static  //defined for all gobal variables
#define internal static  //defines a function as being local to the file (translation unit) it's in

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

//this is a precursor for future OS's that don't do a good job
//of telling what thread you're in when you're multi-threaded
struct thread_context
{
    int placeHolder;
};

#if HANDMADE_INTERNAL
struct debug_read_file_result
{
    uint32 contentsSize;
    void* contents;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context* thread, char* fileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context* thread, void* memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context* thread, char* fileName, uint32 memorySize, void* memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

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
	game_button_state buttons[12];
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

	    //
	    game_button_state terminator;
	};
    };
};

struct game_input
{
    game_button_state mouseButtons[5];
    int32 mouseX, mouseY, mouseZ;

    real32 dTime;
    game_controller_input controllers[5];
};
inline game_controller_input *GetController(game_input* input, int unsigned controllerIndex)
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

    debug_platform_read_entire_file* DEBUGPlatformReadEntireFile;
    debug_platform_free_file_memory* DEBUGPlatformFreeFileMemory;
    debug_platform_write_entire_file* DEBUGPlatformWriteEntireFile;
};

#define GAME_UPDATE_AND_RENDER(name) void name(thread_context* thread, game_memory* memory, game_input* input, game_offscreen_buffer* buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_GET_SOUND_SAMPLES(name) void name(thread_context* thread, game_memory* memory, game_sound_output_buffer* soundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);


//
//
//

struct canonical_position
{
    int32 tileMapX;
    int32 tileMapY;

    int32 tileX;
    int32 tileY;

    //tile relative x and y
    real32 x;
    real32 y;
};

struct raw_position
{
    int32 tileMapX;
    int32 tileMapY;

    //world relative x and y
    real32 x;
    real32 y;
};

struct tile_map
{
    uint32* tiles;
};

struct world
{
    int32 countX;
    int32 countY;
    
    real32 upperLeftX;
    real32 upperLeftY;
    real32 tileWidth;
    real32 tileHeight;

    int32 tileMapCountX;
    int32 tileMapCountY;
    
    tile_map* tileMaps;
};

struct game_state
{
    int32 playerTileMapX;
    int32 playerTileMapY;
    
    real32 playerX;
    real32 playerY;
};

#define HANDMADE_H
#endif


