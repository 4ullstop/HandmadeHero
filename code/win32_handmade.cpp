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

//this is a global for now
global_variable bool running; //static also initializes all variables defined with it to 0
global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytesPerPixel;

internal void
Win32RenderGradient(int xOffset, int yOffset)
{
    int pitch = bitmapWidth * bytesPerPixel;
    uint8* row = (uint8*)bitmapMemory;
    for (int y = 0; y < bitmapHeight; ++y)
    {
	uint8* pixel = (uint8*)row;
	for (int x = 0; x < bitmapWidth; ++x)
	{
	    *pixel = (uint8)(x + xOffset);
	    ++pixel;

	    *pixel = (uint8)(y + yOffset);
	    ++pixel;

	    *pixel = 0;
	    ++pixel;

	    *pixel = 0;
	    ++pixel;
	}
	row += pitch;
    }
}

internal void
Win32ResizeDIBSection(int width, int height)
{
    //free our dib section
    if (bitmapMemory)
    {
	VirtualFree(bitmapMemory, 0, MEM_RELEASE);
    }

    bitmapWidth = width;
    bitmapHeight = height;
    
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = bitmapWidth;
    bitmapInfo.bmiHeader.biHeight = -bitmapHeight; //negative so we have a top down DIB
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    bytesPerPixel = 4;
    
    int bitmapMemorySize = (bitmapWidth * bitmapHeight) *  bytesPerPixel;
    bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Win32RenderGradient(128, 128);
}

internal void
Win32UpdateWindow(HDC deviceContext, RECT* windowRect, int x, int y, int width, int height)
{
    //rectangle to rectangle copy
    int windowWidth = windowRect->right - windowRect->left;
    int windowHeight = windowRect->bottom - windowRect->top;
    StretchDIBits(deviceContext,
		  0, 0, bitmapWidth, bitmapHeight,
		  0, 0, windowWidth, windowHeight,		  
		  bitmapMemory,
		  &bitmapInfo,
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
	RECT clientRect;
	GetClientRect(hwnd, &clientRect); //just gives us the part of the window we can draw into
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
	//producing the width and height for the buffer of the window
	Win32ResizeDIBSection(width, height);
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
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	Win32UpdateWindow(deviceContext, &clientRect, X, Y, WIDTH, HEIGHT);
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
    WNDCLASS windowClass = {};
    windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
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

		Win32RenderGradient(xOffset, yOffset);
		HDC deviceContext = GetDC(windowHandle);

		RECT clientRect;
		GetClientRect(windowHandle , &clientRect);
		int windowWidth = clientRect.right - clientRect.left;
		int windowHeight = clientRect.bottom - clientRect.top;
		
		Win32UpdateWindow(deviceContext, &clientRect, 0, 0, windowWidth, windowHeight);
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
