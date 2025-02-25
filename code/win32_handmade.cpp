#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

#define local_persist static  //locally created variable - exist after creation
#define global_variable static  //defined for all gobal variables
#define internal static  //defines a function as being local to the file (translation unit) it's in

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

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

internal void
Win32RenderGradient(win32_offscreen_buffer* buffer, int xOffset, int yOffset)
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

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow)
{
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
	    HDC deviceContext = GetDC(windowHandle);
	    running = true;
	    //GetMessage pulls the messages off of our queue
	    int xOffset = 0;
	    int yOffset = 0;

	    
	    int samplesPerSecond = 48000;
	    int hz = 256;
	    uint32 runningSampleIndex = 0;
	    int squareWavePeriod = samplesPerSecond / hz;
	    int halfSquareWavePeriod = squareWavePeriod / 2;
	    int bytesPerSample = sizeof(int16)*2;
	    int toneVolume = 5000;
	    int secondaryBufferSize = samplesPerSecond * bytesPerSample;
		
	    Win32InitDSound(windowHandle, samplesPerSecond, samplesPerSecond*bytesPerSample);

	    secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
	    while(running)
	    {


		MSG message;
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{

		    TranslateMessage(&message);
		    DispatchMessage(&message);
		}

		for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex)
		{
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
			bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
			bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
			bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			bool aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
			bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
			bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);
			bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

			int16 stickX = pad->sThumbLX;
			int16 stickY = pad->sThumbLY;

			if (aButton)
			{
			    yOffset += 2;
			}

		    }
		    else
		    {
			//the controller is not available
		    }
		}
		
		Win32RenderGradient(&globalBackBuffer, xOffset, yOffset);


		//Direct sound output test
		DWORD playCursor;
		DWORD writeCursor;
		if (SUCCEEDED(secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
		{
		    DWORD byteToLock = runningSampleIndex*bytesPerSample % secondaryBufferSize;
		    DWORD bytesToWrite;
		    if (byteToLock > playCursor)
		    {
			bytesToWrite = (secondaryBufferSize - byteToLock);
			bytesToWrite += playCursor;
		    }
		    else
		    {
			bytesToWrite = playCursor - byteToLock;
		    }


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
			int16* sampleOut = (int16*)region1;
			DWORD region1SampleCount = region1Size / bytesPerSample;
			DWORD region2SampleCount = region2Size / bytesPerSample;
			for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex)
			{
			    int16 sampleValue = ((runningSampleIndex++ / halfSquareWavePeriod) % 2) ? toneVolume : -toneVolume;
			    *sampleOut++ = sampleValue;
			    *sampleOut++ = sampleValue;
			}

			sampleOut = (int16*)region2;
			for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex)
			{
			    int16 sampleValue = ((runningSampleIndex++ / halfSquareWavePeriod) % 2) ? toneVolume : -toneVolume;
			    *sampleOut++ = sampleValue;
			    *sampleOut++ = sampleValue;
			}
		    }
		}
		win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
		Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, 0, 0, dimension.width, dimension.height);

		
		++xOffset;
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
