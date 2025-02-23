#include <windows.h>
#include <stdint.h>
#include <xinput.h>



#define local_perisist static
#define global_variable static
#define internal static

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

global_variable bool running;
global_variable win32_offscreen_buffer globalBackBuffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNTECTED);
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWOR dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state* XInputSetState = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32InitDSound(HWND window, int32 samplesPerSecond, int32 bufferSize)
{
    HMODULE dSoundLibrary = LoadLibrary("dsound.dll");
    if (dSoundLibrary)
    {
	direct_sound_create* directSoundCreate = (direct_sound_create*)GetProcAddress(dSoundLibrary, "DirectSoundCreate");
    }

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

	if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
	{
	    DSBUFFERDESC bufferDescription = {};
	    bufferDescription.dwSize = sizeof(bufferDescription);
	    bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

	    LPDIRECTSOUNDBUFFER primaryBuffer;
	    if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
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
	LPDIRECTSOUNDBUFFER secondaryBuffer;
	if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &secondaryBuffer, 0)))
	{
	    OutputDebugStringA("Secondary buffer format was set\n");
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
    HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
    if (!XInputLibrary)
    {
	XInputLibrfary = LoadLibrary("xinput1_3.dll");
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

internal void
Win32ResizeDIBSeciton(win32_offscreen_buffer* buffer, int width, int height)
{
    if (buffer->memory)
    {
	VirutalFree(buffer->memory, 0, MEM_RELEASE);
    }
    buffer->width = width;
    buffer->height = height;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    buffer->bytesPerPixel = 4;

    int bitmapMemorySize = (buffer->width * buffer->height0) * buffer->bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    buffer->pitch = width * buffer->bytesPerPixel;
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
Win32DisplayBufferWindow(win32_offscreen_buffer* buffer,
			 HDC deviceContext,
			 int x, int y,
			 int windowWidth, int windowHeight)
{
    StretchDIBits(deviceContext,
		  0, 0, windowWidth, windowHeight,
		  0, 0, buffer->width, buffer->height,
		  buffer->memory,
		  &buffer->info,
		  DIB_RGB_COLORS,
		  SRCOPY);
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

	    }
	    else if (VKCode == VK_SPACE)
	    {

	    }
	}
	bool32 altKeyWasDown = ((lParam & (1 << 29)) != 0);
	if ((VKCode == VK_F4) && altKeyWasDown)
	{
	    running = false;
	}
    } break;
    case WM_PAINT:
    {
	PAINTSTRUCT paint;
	HDC deviceContext = BeginPaint(hwnd, &paint);
	
	int height = paint.rcPaint.bottom - paint.rcPaint.top;
	int width = paint.rcPaint.right - paint.rcPaint.left;
	int x = paint.rcPaint.left;
	int y = paint.rcPaint.top;

	win32_window_dimension dimension = Win32GetWindowDimension(hwnd);
	
	Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, x, y, dimension.width, dimension.height);
	EndPaint(hwnd, &paint);
	    
    } break;
    default:
    {
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
    windowClass.lpszClassName = "Handmade Hero Window Class";

    if (RegisterClass(&windowClass))
    {
	HWND windowHandle = CreateWindowEx(
	    0,
	    windowClass.lpszClassName,
	    "Handmade Hero",
	    WS_OVERLAPPEDWINDOW|WS_VISIBLE,
	    CW_USEDEFAULT,
	    CW_USEDEFAULT,
	    CW_USEDEFAULT,
	    CW_USEDEFAULT,
	    0,
	    0,
	    hInstance,
	    0);

	if (windowHandle)
	{
	    HDC deviceContext = GetDC(windowHandle);
	    running = true;

	    int xOffset = 0;
	    int yOffset = 0;

	    Win32InitDSound(windowHandle, 48000, 48000*sizeof(int16)*2);
	    while (running)
	    {
		MSG message;
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
		    TranslateMessage(&message);
		    DispatchMessage(&message);
		}

		for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex)
		{
		    XINPUT_STATE controllerState;

		    if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
		    {
			XINPUT_GAMEPAD* pad = &controllerState.Gamepad;

			bool Up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
			bool Down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
			bool Left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
			bool Right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
			bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
			bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
			bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOUDLER);
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
		}
		Win32RenderGradient(&globalBackBuffer, xOffset, yOffset);

		win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
		Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, 0, 0, dimension.width, dimension.height);

		++xOffset;
	    }
	}
    }
}
