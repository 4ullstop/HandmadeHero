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
#include <stdio.h>

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
global_variable int64 perfCountFrequency;
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
	uint32 VKCode = (uint32)wParam;
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

internal void
Win32ProcessKeyboardMessage(game_button_state* newState, bool32 isDown)
{
    //should only be getting this message if the state changed
    Assert(newState->endedDown != isDown);
    newState->endedDown = isDown;
    ++newState->halfTransitionCount;
}

internal void
Win32ProcessPendingMessages(game_controller_input* keyboardController)
{
    MSG message; 
    
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
	switch(message.message)
	{
	case WM_QUIT:
	{
	    running = false;
	} break;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
	    uint32 VKCode = (uint32)message.wParam;
	    bool32 wasDown = ((message.lParam & (1 << 30)) != 0);
	    bool32 isDown = ((message.lParam & (1 << 31)) == 0);
	    //we do not process keyboard events that were already down
	    if (wasDown != isDown)
	    {
		if (VKCode == 'W')
		{
		    Win32ProcessKeyboardMessage(&keyboardController->moveUp,
						isDown);
		}
		else if (VKCode == 'A')
		{
		    Win32ProcessKeyboardMessage(&keyboardController->moveLeft,
						isDown);
		}
		else if (VKCode == 'S')
		{
		    Win32ProcessKeyboardMessage(&keyboardController->moveDown,
						isDown);
		}
		else if (VKCode == 'D')
		{
		    Win32ProcessKeyboardMessage(&keyboardController->moveRight,
						isDown);
		}
		else if (VKCode == 'Q')
		{
		    Win32ProcessKeyboardMessage(&keyboardController->leftShoulder,
						isDown);
		}
		else if (VKCode == 'E')
		{
		    Win32ProcessKeyboardMessage(&keyboardController->rightShoulder,
						isDown);
		}
		else if (VKCode == VK_UP)
		{
		    Win32ProcessKeyboardMessage(&keyboardController->actionUp,
						isDown);
		}
		else if (VKCode == VK_DOWN)
		{
		    Win32ProcessKeyboardMessage(&keyboardController->actionDown,
						isDown);
		}
		else if (VKCode == VK_LEFT)
		{
		    Win32ProcessKeyboardMessage(&keyboardController->actionLeft,
						isDown);

		}
		else if (VKCode == VK_RIGHT)
		{
		    Win32ProcessKeyboardMessage(&keyboardController->actionRight,
						isDown);
		}
		else if (VKCode == VK_ESCAPE)
		{
		    running = false;
		}
		else if (VKCode == VK_SPACE)
		{

		}
	    }
	    bool32 altKeyWasDown = ((message.lParam & (1 << 29)) != 0);
	    if ((VKCode == VK_F4) && altKeyWasDown)
	    {
		running = false;
	    }
	} break;
	default:
	{
	    TranslateMessage(&message);
	    DispatchMessage(&message);
	} break;
	}
    }
}

internal real32
Win32ProcessInputStickValue(SHORT value, SHORT deadZoneThreshold)
{
    real32 result = 0;
    
    if (value < -deadZoneThreshold)
    {
	result = (real32)((value + deadZoneThreshold) / (32768.0f - deadZoneThreshold));
    }
    else if (value > deadZoneThreshold)
    {
	result = (real32)((value - deadZoneThreshold) / (32767.0f - deadZoneThreshold));
    }
    return(result);
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return(result);
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    real32 result = ((real32)(end.QuadPart - start.QuadPart) /
				    (real32)perfCountFrequency);
    return(result);
}

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow)
{
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    perfCountFrequency = perfCountFrequencyResult.QuadPart;

    //Setting the windows scheduler granularity to 1ms so that
    // our Sleep() can be more granular.
    UINT desiredSchedulerMs = 1;
    bool32 sleepIsGranular = (timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR);

	
    
    Win32LoadXInput();
    Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);
			  
    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    windowClass.lpfnWndProc = Win32MainWindowProc;
    windowClass.hInstance = hInstance;
//    windowClass.hIcon;
    windowClass.lpszClassName = "Handmade Hero Window Class";

    //TODO: how do we reliably query this on windows
    int monitorRefreshHz = 60;
    int gameUpdateHz = monitorRefreshHz / 2;
    real32 targetSecondsPerFrame = 1.0f / (real32)gameUpdateHz;
    
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
	    gameMemory.transientStorageSize = Gigabytes(1);
	    
	    uint64 totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
	    gameMemory.permanentStorage = VirtualAlloc(baseAddress, (size_t)totalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	    gameMemory.transientStorage = ((uint8*)gameMemory.permanentStorage + gameMemory.permanentStorageSize);

	    
	    
	    HDC deviceContext = GetDC(windowHandle);
	    running = true;
	    //GetMessage pulls the messages off of our queue
	    
	    bool32 soundIsPlaying = false;
	    




	    if (samples && gameMemory.permanentStorage && gameMemory.transientStorage)
	    {
		game_input input[2] = {};
		game_input* newInput = &input[0];
		game_input* oldInput = &input[1];

		LARGE_INTEGER lastCounter = Win32GetWallClock();
		uint64 lastCycleCount = __rdtsc();

		/*
		  Start of the main game loop
		  
		 */
		while(running)
		{
		    game_controller_input* oldKeyboardController = GetController(oldInput, 0);
		    game_controller_input* newKeyboardController = GetController(newInput, 0);
		    *newKeyboardController = {};
		    newKeyboardController->isConnected = true;

		    for (int buttonIndex = 0; buttonIndex < ArrayCount(newKeyboardController->buttons); ++buttonIndex)
		    {
			newKeyboardController->buttons[buttonIndex].endedDown =
			    oldKeyboardController->buttons[buttonIndex].endedDown;
		    }
		    
		    Win32ProcessPendingMessages(newKeyboardController);

		    DWORD maxControllerCount = XUSER_MAX_COUNT;
		    if (maxControllerCount > (ArrayCount(newInput->controllers)) - 1)
		    {
			maxControllerCount = (ArrayCount(newInput->controllers)) - 1;
		    }
		
		    for (DWORD controllerIndex = 0; controllerIndex < maxControllerCount; ++controllerIndex)
		    {

			int ourControllerIndex = controllerIndex + 1;
			game_controller_input* oldController = GetController(oldInput, ourControllerIndex);
			game_controller_input* newController = GetController(newInput, ourControllerIndex);
			XINPUT_STATE controllerState;

			if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
			{
			    newController->isConnected = true;   
			    //the controller is plugged in
			    // controllerState.dwPacketNumber increments too frequently
			    XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

			    newController->isAnalog = true;
			    newController->stickAverageX = Win32ProcessInputStickValue(pad->sThumbLX,
								   XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			    newController->stickAverageY = Win32ProcessInputStickValue(pad->sThumbLY,
								   XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

			    if ((newController->stickAverageX != 0.0f) ||
				(newController->stickAverageY != 0.0f))
			    {
				newController->isAnalog = true;
			    }
			    if (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
			    {
				newController->stickAverageY = 1.0f;
				newController->isAnalog = false;
			    }
			    if (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
			    {
				newController->stickAverageY = -1.0f;
				newController->isAnalog = false;
			    }
			    if (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
 			    {
				newController->stickAverageX = -1.0f;
				newController->isAnalog = false;
			    }
			    if (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
			    {
				newController->stickAverageX = 1.0f;
				newController->isAnalog = false;
			    }

			    real32 threshold = 0.5f; 
			    Win32ProcessXInputDigitalButton((newController->stickAverageX < -threshold) ? 1 : 0,
							    &oldController->moveLeft, 1,
							    &newController->moveLeft);
			    Win32ProcessXInputDigitalButton((newController->stickAverageX > threshold) ? 1 : 0,
							    &oldController->moveRight, 1,
							    &newController->moveRight);
			    Win32ProcessXInputDigitalButton((newController->stickAverageY < -threshold) ? 1 : 0,
							    &oldController->moveDown, 1,
							    &newController->moveDown);
			    Win32ProcessXInputDigitalButton((newController->stickAverageY < threshold) ? 1 : 0,
							    &oldController->moveUp, 1,
							    &newController->moveUp);

			    
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->actionDown, XINPUT_GAMEPAD_A,
							    &newController->actionDown);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->actionRight, XINPUT_GAMEPAD_B,
							    &newController->actionRight);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->actionLeft, XINPUT_GAMEPAD_X,
							    &newController->actionLeft);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->actionUp, XINPUT_GAMEPAD_Y,
							    &newController->actionUp);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->leftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
							    &newController->leftShoulder);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->rightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
							    &newController->rightShoulder);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							    &oldController->back,
							    XINPUT_GAMEPAD_BACK,
							    &newController->back);
			    Win32ProcessXInputDigitalButton(pad->wButtons,
							     &oldController->start,
							     XINPUT_GAMEPAD_START,
							     &newController->start); 
							    

 
			}
			else
			{
			    //the controller is not available
			    newController->isConnected = false; 
			}
		    }

		    DWORD playCursor = 0;
		    DWORD writeCursor = 0;
		    DWORD targetCursor = 0;
		    DWORD bytesToWrite = 0;
		    DWORD byteToLock = 0;
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
		
		    LARGE_INTEGER workCounter = Win32GetWallClock();
		    real32 workSecondsElapsed = Win32GetSecondsElapsed(lastCounter, workCounter);


		    real32 secondsElapsedForFrame = workSecondsElapsed;
		    if (secondsElapsedForFrame < targetSecondsPerFrame)
		    {
			while (secondsElapsedForFrame < targetSecondsPerFrame)
			{
			    //how many ms are left to wait
			    if (sleepIsGranular)
			    {
				DWORD sleepMs = (DWORD) (1000.0f * (targetSecondsPerFrame - secondsElapsedForFrame));
				Sleep(sleepMs);
			    }
			    secondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter,
									    Win32GetWallClock());
			}
		    }
		    else
		    {
			//if the secondsElapsedForFrame is higher than the target
			// then we missed a frame
		    }

		    win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
		    Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, 0, 0, dimension.width, dimension.height);
		    LARGE_INTEGER endCounter = Win32GetWallClock();
		    
		    uint64 endCycleCount = __rdtsc();
		    uint64 cyclesElapsed = endCycleCount - lastCycleCount;
		    //Milliseconds per frame
		    int32 msPerFrame = (int32)(1000.0f * (Win32GetSecondsElapsed(lastCounter, endCounter)));
		    int32 FPS = 0;
		    int32 MCPF =  (int32)(cyclesElapsed / (1000 * 1000));


		    char fpsBuffer[250];
		    sprintf_s(fpsBuffer, "%dms/f, %df/s, %dmc/f\n", msPerFrame, FPS, MCPF);
		    OutputDebugStringA(fpsBuffer);  


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
 
