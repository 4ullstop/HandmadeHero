#include <windows.h>
#include <stdint.h>

#define local_persist static  //locally created variable - exist after creation
#define global_variable static  //defined for all gobal variables
#define internal static  //defines a function as being local to the file (translation unit) it's in

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

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

//this is a global for now
global_variable bool running; //static also initializes all variables defined with it to 0
global_variable win32_offscreen_buffer globalBackBuffer;

struct win32_window_dimension
{
    int width;
    int height;
};

win32_window_dimension Win32GetWindowDimension(HWND window)
{
    win32_window_dimension result;
    RECT clientRect;
    GetClientRect(window, &clientRect);
    result.width = clientRect.right - clientRect.left;
    result.height = clientRect.bottom - clientRect.top;

    return(result);
}

internal void
Win32RenderGradient(win32_offscreen_buffer buffer, int xOffset, int yOffset)
{
    uint8* row = (uint8*)buffer.memory;
    for (int y = 0; y < buffer.height; ++y)
    {
	uint32* pixel = (uint32*)row;
	for (int x = 0; x < buffer.width; ++x)
	{
	    uint8 blue = (x + xOffset);
	    uint8 green = (y + yOffset);
	    
	    *pixel++ = ((green << 8) | blue);

	}
	row += buffer.pitch;
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
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    buffer->pitch = width * buffer->bytesPerPixel;
}

internal void
Win32DisplayBufferWindow(win32_offscreen_buffer buffer,
			 HDC deviceContext,
			 int x, int y,
			 int windowWidth, int windowHeight)
{
    //rectangle to rectangle copy
    StretchDIBits(deviceContext,
		  0, 0, windowWidth, windowHeight,
		  0, 0, buffer.width, buffer.height,
		  buffer.memory,
		  &buffer.info,
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
    case WM_PAINT:
    {
	PAINTSTRUCT paint;
	HDC deviceContext = BeginPaint(hwnd, &paint);

	//To get the window size we actually have to do the calculations ourselves
	int HEIGHT = paint.rcPaint.bottom  - paint.rcPaint.top;
	int WIDTH = paint.rcPaint.right - paint.rcPaint.left;
	int X = paint.rcPaint.left;
	int Y = paint.rcPaint.top;

	win32_window_dimension dimension = Win32GetWindowDimension(hwnd);
	
	Win32DisplayBufferWindow(globalBackBuffer, deviceContext, X, Y, dimension.width, dimension.height);
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

    Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);
			  
    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW|CS_VREDRAW;
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
	    running = true;
	    //GetMessage pulls the messages off of our queue
	    int xOffset = 0;
	    int yOffset = 0;

	    while(running)
	    {


		MSG message;
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
		    if (message.message == WM_QUIT) running = false;
		    TranslateMessage(&message);
		    DispatchMessage(&message);
		}

		Win32RenderGradient(globalBackBuffer, xOffset, yOffset);
		HDC deviceContext = GetDC(windowHandle);

		win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
		Win32DisplayBufferWindow(globalBackBuffer, deviceContext, 0, 0, dimension.width, dimension.height);
		ReleaseDC(windowHandle, deviceContext);
		
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
