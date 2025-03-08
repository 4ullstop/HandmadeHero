/*
  This is not a final platform layer

  - Saved game locations
  - Getting a handle to our own executable file
  - Asset loading path
  - Threading (how to launch a thread)
  - Raw input (support for multiple keyboards)
  - Sleep/timeBeginPeriod
  - ClipCursor() for multimonitor support
  - fullscreen support
  - WM_SETCURSOR (control cursor visibility)
  - QueryCancelAutoplay
  - WM_ACTIVATEAPP (for when we are not the active application)
  - Blit speed improvements (BitBlit)
  - Hardware acceleration (OpenGL or Direct3D)
  - GetKeyboardLayout (for french keyboards, international WASD support)

  Just a partial list of stuff
*/
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

#include <math.h>
#include "handmade.h"
#include "handmade.cpp"


#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <malloc.h>

#include "win32_handmade.h"

//this is a global for now
global_variable bool running; //static also initializes all variables defined with it to 0
global_variable win32_offscreen_buffer globalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER secondaryBuffer;
//known as an efficient way to define function pointers so that we don't have to link
//things properly
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal debug_read_file_result
DEBUGPlatformReadEntireFile(char* fileName)
{
    debug_read_file_result result = {};
    HANDLE fileHandle = CreateFile(fileName,
				   GENERIC_READ,
				   FILE_SHARE_READ,
				   0,
				   OPEN_EXISTING,
				   0,
				   0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(fileHandle, &fileSize))
	{
	    uint32 fileSize32 = SafeTruncateUInt64(fileSize.QuadPart);
	    result.contents = VirtualAlloc(0, fileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	    if (result.contents)
	    {
		DWORD bytesRead;
		if(ReadFile(fileHandle,
			    result.contents,
			    fileSize32,
			    &bytesRead,
			    0) &&
		   (fileSize32 == bytesRead))
		{
		    result.contentsSize = fileSize32;
		}
		else
		{
		    DEBUGPlatformFreeFileMemory(result.contents);
		    result.contents = 0;
		}
	    }
	    else
	    {

	    }
	}
	else
	{

	}
	CloseHandle(fileHandle);
    }
    else
    {

    }
    return(result);
}

internal void
DEBUGPlatformFreeFileMemory(void* memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

internal bool32
DEBUGPlatformWriteEntireFile(char* fileName, uint32 memorySize, void* memory)
{
    bool32 result = 0;
    HANDLE fileHandle = CreateFile(fileName,
				   GENERIC_WRITE,
				   0,
				   0,
				   CREATE_ALWAYS,
				   0,
				   0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
	DWORD bytesWritten;
	if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
	{
	    result = (bytesWritten == memorySize);
	}
	else
	{

	}
	CloseHandle(fileHandle);
    }
    return(result);
}

internal void
Win32InitDSound(HWND window, int32 samplesPerSecond, int32 bufferSize)
{
    //load the library
    //get direct sound object
    //Create a primary buffer
    //Create a secondary buffer (the thing we actually write to)
    //Start it playing

    HMODULE dSoundLibrary = LoadLibrary("dsound.dll");
    if (dSoundLibrary)
    {
	direct_sound_create* directSoundCreate = (direct_sound_create*)GetProcAddress(dSoundLibrary, "DirectSoundCreate");

	LPDIRECTSOUND directSound;
	if (directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0)))
	{
	    WAVEFORMATEX waveFormat = {};
	    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	    waveFormat.nChannels = 2;
	    waveFormat.nSamplesPerSec = samplesPerSecond;
	    waveFormat.wBitsPerSample = 16;
	    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
	    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	    waveFormat.cbSize = 0;
	    
	    if(SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
	    {
		DSBUFFERDESC bufferDescription = {};
		bufferDescription.dwSize = sizeof(bufferDescription);
		bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

		
		LPDIRECTSOUNDBUFFER primaryBuffer;
		if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
		{
		    if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
		    {
			OutputDebugStringA("Primary buffer format was set\n");
		    }
		    else
		    {

		    }
		}
		else
		{
		    
		}
	    }
	    else
	    {
		
	    }

	    DSBUFFERDESC bufferDescription = {};
	    bufferDescription.dwSize = sizeof(bufferDescription);
	    bufferDescription.dwFlags = 0;
	    bufferDescription.dwBufferBytes = bufferSize;
	    bufferDescription.lpwfxFormat = &waveFormat;

	    if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &secondaryBuffer, 0)))
	    {
		OutputDebugStringA("Secondary buffer format was set\n");
	    }
	}
	else
	{
	    
	}
    }
    else
    {
	
    }
	
}

internal void
Win32LoadXInput(void)
{
    //Load the dll we actually want for XInput
    HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
    if (!XInputLibrary)
    {
	XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    if (!XInputLibrary)
    {
	XInputLibrary = LoadLibrary("xinput1_3.dll");
    }
    if (XInputLibrary)
    {
	XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
	XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else
    {

    }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND window)
{
    win32_window_dimension result;
    RECT clientRect;
    GetClientRect(window, &clientRect);
    result.width = clientRect.right - clientRect.left;
    result.height = clientRect.bottom - clientRect.top;

    return(result);
}

//the allocation of our backbuffer which we will continuously write to
internal void
Win32ResizeDIBSection(win32_offscreen_buffer* buffer, int width, int height)
{
    //free our dib section
    if (buffer->memory)
    {
	VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height; //negative so we have a top down DIB
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    buffer->bytesPerPixel = 4;
    
    int bitmapMemorySize = (buffer->width * buffer->height) *  buffer->bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    buffer->pitch = width * buffer->bytesPerPixel;
}

internal void
Win32DisplayBufferWindow(win32_offscreen_buffer* buffer,
			 HDC deviceContext,
			 int x, int y,
			 int windowWidth, int windowHeight)
{
    //rectangle to rectangle copy
    StretchDIBits(deviceContext,
		  0, 0, windowWidth, windowHeight,
		  0, 0, buffer->width, buffer->height,
		  buffer->memory,
		  &buffer->info,
		  DIB_RGB_COLORS,
		  SRCCOPY);//what kind of bitwise operations you want to do
	
}

LRESULT CALLBACK Win32MainWindowProc(HWND hwnd,
				UINT uMsg,
				WPARAM wParam,
				LPARAM lParam)
{
    LRESULT result = 0;
    switch(uMsg)
    {
    case WM_SIZE:
    {
	OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY:
    {
	running = false;
	OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE:
    {
	running = false;
	OutputDebugStringA("WM_CLOSE\n");
    } break;
    case WM_ACTIVATEAPP:
    {
	OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
	uint32 VKCode = wParam;
	bool wasDown = ((lParam & (1 << 30)) != 0);
	bool isDown = ((lParam & (1 << 31)) == 0);
	if (wasDown != isDown)
	{
	    if (VKCode == 'W')
	    {

	    }
	    else if (VKCode == 'A')
	    {
		
	    }
	    else if (VKCode == 'S')
	    {
		
	    }
	    else if (VKCode == 'D')
	    {
		
	    }
	    else if (VKCode == 'Q')
	    {
		
	    }
	    else if (VKCode == 'E')
	    {
		
	    }
	    else if (VKCode == VK_UP)
	    {
		
	    }
	    else if (VKCode == VK_DOWN)
	    {
		
	    }
	    else if (VKCode == VK_LEFT)
	    {
		
	    }
	    else if (VKCode == VK_RIGHT)
	    {
		
	    }
	    else if (VKCode == VK_ESCAPE) 
	    {
		running = false;
	    }
	    else if (VKCode == VK_SPACE) 
	    {
		
	    }
	}
	bool32 altKeyWasDown = ((lParam & (1 << 29)) != 0); //is this bit down?
	if ((VKCode == VK_F4) && altKeyWasDown)
	{
	    running = false;
	}

    } break;
    case WM_PAINT:
    {
	PAINTSTRUCT paint;
	HDC deviceContext = BeginPaint(hwnd, &paint);
 
	//To get the window size we actually have to do the calculations ourselves
	int X = paint.rcPaint.left;
	int Y = paint.rcPaint.top;

	win32_window_dimension dimension = Win32GetWindowDimension(hwnd);
	
	Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, X, Y, dimension.width, dimension.height);
	EndPaint(hwnd, &paint);
    } break;
    default:
    {
	//if you don't want to worry about a particular message, windows can handle it
	result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    } break;
    }
    return result;
} 
 
internal void
Win32ClearBuffer(win32_sound_output* soundOutput)
{
    VOID* region1;
    DWORD region1Size;

    VOID* region2;
    DWORD region2Size;
    
    if (SUCCEEDED(secondaryBuffer->Lock(0, soundOutput->secondaryBufferSize,
					&region1, &region1Size,
					&region2, &region2Size,
					0)))
    {
	uint8* destSample = (uint8*)region1;
	for (DWORD byteIndex = 0; byteIndex < region1Size; ++byteIndex)
	{
	    *destSample++ = 0;
	}

	destSample = (uint8*)region2;
	for (DWORD byteIndex = 0; byteIndex < region2Size; ++byteIndex)
	{
	    *destSample++ = 0;
	}

	secondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
    }
}

internal void
Win32FillSoundBuffer(win32_sound_output* soundOutput, DWORD byteToLock, DWORD bytesToWrite, game_sound_output_buffer* sourceBuffer)
{
    VOID* region1;
    DWORD region1Size;
    
    VOID* region2;
    DWORD region2Size;
    
    if(SUCCEEDED(secondaryBuffer->Lock(byteToLock,
				       bytesToWrite,
				       &region1,
				       &region1Size,
				       &region2,
				       &region2Size,
				       0)))
    {
	//assert
	int16* destSample = (int16*)region1;
	int16* sourceSample = sourceBuffer->samples;
	DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;
	for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex)
	{
	    *destSample++ = *sourceSample++;
	    *destSample++ = *sourceSample++;
 
	    ++soundOutput->runningSampleIndex;
	}
	
	destSample = (int16*)region2;
	DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
	for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex)
	{
	    *destSample++ = *sourceSample++;
	    *destSample++ = *sourceSample++;

	    ++soundOutput->runningSampleIndex;
	}
	
	secondaryBuffer->Unlock(region1, region1Size,
				region2, region2Size);
    }
    
}

internal void
Win32ProcessXInputDigitalButton(DWORD xInputButtonState, game_button_state* oldState, DWORD buttonBit, game_button_state* newState)
{
    newState->endedDown = ((xInputButtonState & buttonBit) == buttonBit);
    newState->halfTransitionCount = (newState->endedDown != oldState->endedDown) ? 1 : 0;
}

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow)
{
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    int64 perfCountFrequency = perfCountFrequencyResult.QuadPart;
    
    
    Win32LoadXInput();
    Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);
			  
    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    windowClass.lpfnWndProc = Win32MainWindowProc;
    windowClass.hInstance = hInstance;
//    windowClass.hIcon;
    windowClass.lpszClassName = "Handmade Hero Window Class";


    
    //Registering the window class
    if (RegisterClass(&windowClass))
    {
	HWND windowHandle = CreateWindowEx(
	    0, //extra window style information
	    windowClass.lpszClassName, //the class instance we created above
	    "Handmade Hero", //name of the window
	    WS_OVERLAPPEDWINDOW|WS_VISIBLE, //window style (bunch of parameters like borderless...etc..
	    CW_USEDEFAULT, //x pos
	    CW_USEDEFAULT, //y pos
	    CW_USEDEFAULT, //width
	    CW_USEDEFAULT, //height
	    0, //windows inside windows
	    0, //no menu
	    hInstance, //our instance
	    0); //passing parameters in to the window which happens in WM_CREATE, can be anything we want
	
	if (windowHandle)
	{
	    
	    win32_sound_output soundOutput = {};
	    
	    soundOutput.samplesPerSecond = 48000;
	    soundOutput.bytesPerSample = sizeof(int16)*2;
	    soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond*soundOutput.bytesPerSample;
	    soundOutput.latencySampleCount = soundOutput.samplesPerSecond / 15;
	    Win32InitDSound(windowHandle, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
	    Win32ClearBuffer(&soundOutput);

	    int16* samples = (int16*)VirtualAlloc(0, soundOutput.secondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	    
	    secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
	    
	    
#if HANDMADE_INTERNAL
	    LPVOID baseAddress = (LPVOID)Terabytes((uint64)2);
#else
	    LPVOID baseAddress = 0;
#endif
	    
	    game_memory gameMemory = {};
	    gameMemory.permanentStorageSize = Megabytes(64);
	    gameMemory.transientStorageSize = Gigabytes((uint64)4);
	    
	    uint64 totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
	    gameMemory.permanentStorage = VirtualAlloc(baseAddress, totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	    gameMemory.transientStorage = ((uint8*)gameMemory.permanentStorage + gameMemory.permanentStorageSize);

	    
	    
	    HDC deviceContext = GetDC(windowHandle);
	    running = true;
	    //GetMessage pulls the messages off of our queue
	    
	    bool32 soundIsPlaying = false;
	    

	    LARGE_INTEGER lastCounter;
	    QueryPerformanceCounter(&lastCounter);

	    if (samples && gameMemory.permanentStorage && gameMemory.transientStorage)
	    {
		
		game_input input[2] = {};
		game_input* newInput = &input[0];
		game_input* oldInput = &input[1];


		
	    
		uint64 lastCycleCount = __rdtsc();
		while(running)
		{
		
		    MSG message;


		
		    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		    {

			TranslateMessage(&message);
			DispatchMessage(&message);
		    }


		    int maxControllerCount = XUSER_MAX_COUNT;
		    if (maxControllerCount > ArrayCount(newInput->controllers))
		    {
			maxControllerCount = ArrayCount(newInput->controllers);
		    }
		
		    for (DWORD controllerIndex = 0; controllerIndex < maxControllerCount; ++controllerIndex)
		    {
			game_controller_input* oldController = &oldInput->controllers[controllerIndex];
			game_controller_input* newController = &newInput->controllers[controllerIndex];
			XINPUT_STATE controllerState;
   
			if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
			{
			    //the controller is plugged in
			    // controllerState.dwPacketNumber increments too frequently
			    XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

			    bool Up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
			    bool Down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			    bool Left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			    bool Right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

			    newController->startX = oldController->endX;
			    newController->startY = oldController->endY;
			    newController->isAnalog = true;
			
			    real32 x;
			    if (pad->sThumbLX < 0)
			    {
				x = (real32)pad->sThumbLX / 32768.0f;
			    }
			    else
			    {
				x = (real32)pad->sThumbLX / 32767.0f;
			    }

			    newController->minX = newController->maxX = newController->endX = x;
			
			    real32 y;
			    if (pad->sThumbLY < 0)
			    {
				y = (real32)pad->sThumbLY / 32768.0f;
			    }
			    else
			    {
				y = (real32)pad->sThumbLY / 32767.0f;
			    }
			    newController->minY = newController->maxY = newController->endY = y;

			

			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->down, XINPUT_GAMEPAD_A,
							    &newController->down);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->right, XINPUT_GAMEPAD_B,
							    &newController->right);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->left, XINPUT_GAMEPAD_X,
							    &newController->left);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->up, XINPUT_GAMEPAD_Y,
							    &newController->up);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->leftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
							    &newController->leftShoulder);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->rightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
							    &newController->rightShoulder);
 
			}
			else
			{
			    //the controller is not available
			}
		    }

		    DWORD playCursor;
		    DWORD writeCursor;
		    DWORD targetCursor;
		    DWORD bytesToWrite;
		    DWORD byteToLock;
		    bool32 soundIsValid = false;
		    if (SUCCEEDED(secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
		    {
			byteToLock = ((soundOutput.runningSampleIndex*soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize);
			targetCursor = ((playCursor + (soundOutput.latencySampleCount*soundOutput.bytesPerSample))% soundOutput.secondaryBufferSize);
		    
			if (byteToLock > targetCursor)
			{
			    bytesToWrite += (soundOutput.secondaryBufferSize - byteToLock);
			    bytesToWrite += targetCursor;
			}
			else
			{
			    bytesToWrite = targetCursor - byteToLock;
			}
		    
			soundIsValid = true;
		    }
		    game_offscreen_buffer buffer = {};
		    buffer.memory = globalBackBuffer.memory;
		    buffer.width = globalBackBuffer.width;
		    buffer.height = globalBackBuffer.height;
		    buffer.pitch = globalBackBuffer.pitch;
		    buffer.bytesPerPixel = globalBackBuffer.bytesPerPixel;


		    game_sound_output_buffer soundBuffer = {};
		    soundBuffer.samplesPerSecond = soundOutput.samplesPerSecond;
		    soundBuffer.sampleCount = bytesToWrite / soundOutput.bytesPerSample;
		    soundBuffer.samples = samples;
		
		
		    GameUpdateAndRender(&gameMemory, newInput, &buffer, &soundBuffer);

		    //Direct sound output test


		    if (soundIsValid)
		    {
			Win32FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite, &soundBuffer);
		    }
 
		    if(!soundIsPlaying)
		    {
			secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
			soundIsPlaying = true;
		    }
		
		    win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
		    Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, 0, 0, dimension.width, dimension.height);

		    uint64 endCycleCount = __rdtsc();
		
		    LARGE_INTEGER endCounter;
		    QueryPerformanceCounter(&endCounter);


		    uint64 cyclesElapsed = endCycleCount - lastCycleCount;
		
		    int64 counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
		    //Milliseconds per frame
		    int32 msPerFrame = (int32)((1000*counterElapsed) / perfCountFrequency);
		    int32 FPS = perfCountFrequency / counterElapsed;
		    int32 MCPF =  (int32)(cyclesElapsed / (1000 * 1000));

#if 0		
		    char buffer[250];
		    wsprintf(buffer, "%dms/f, %df/s, %dmc/f\n", msPerFrame, FPS, MCPF);
		    OutputDebugStringA(buffer);  
#endif		
		    lastCounter = endCounter;
		    lastCycleCount = endCycleCount;

		    game_input* temp = newInput;
		    newInput = oldInput;
		    oldInput = temp;
		}
	    }
	    else
	    {

	    }
	}
	else
	{

	}
    }
    else
    {
	
    }
    
    return(0);
}
 
