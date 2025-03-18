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
    
    DWORD flipPlayCursor;
    DWORD flipWriteCursor;
};

#define WIN32_HANDMADE_H
#endif
