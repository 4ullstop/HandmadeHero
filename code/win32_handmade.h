#if !defined(WIN32_HANDMADE_H)


struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void* memory;
    int height;
    int width;
    int pitch;
    int bytesPerPixel;
};

struct win32_window_dimension
{
    int width;
    int height;
};

struct win32_sound_output
{
    int samplesPerSecond;
    uint32 runningSampleIndex;
    int bytesPerSample;
    DWORD secondaryBufferSize;
    real32 tSine;
    int latencySampleCount;
    DWORD safetyBytes; 
    //add bytesPerSecond field
};

struct win32_debug_time_marker
{
    DWORD outputPlayCursor;
    DWORD outputWriteCursor;
    DWORD outputLocation;
    DWORD outputByteCount;
    DWORD expectedFlipPlayCursor;

    DWORD flipPlayCursor;
    DWORD flipWriteCursor;
};

struct win32_game_code
{
    HMODULE gameCodeDLL; //saving this so we can use it to free old stuff later
    FILETIME dllLastWriteTime;
    game_update_and_render* UpdateAndRender;
    game_get_sound_samples* GetSoundSamples;

    bool32 isValid;
};
#define WIN32_HANDMADE_H
#endif
