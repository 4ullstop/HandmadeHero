#include <windows.h>

#define local_persist static  //locally created variable - exist after creation
#define global_variable static  //defined for all gobal variables
#define internal static  //defines a function as being local to the file (translation unit) it's in

//this is a global for now
global_variable bool running; //static also initializes all variables defined with it to 0
global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMemory;
global_variable HBITMAP bitmapHandle;
global_variable HDC bitmapDeviceContext;

internal void
Win32ResizeDIBSection(int width, int height)
{
    //free our dib section
    if (bitmapHandle)
    {
	DeleteObject(bitmapHandle);
    }

    if (!bitmapDeviceContext)
    {
	bitmapDeviceContext = CreateCompatibleDC(0);
    }

    
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;


    
    bitmapHandle = CreateDIBSection(
	bitmapDeviceContext,
	&bitmapInfo,
	DIB_RGB_COLORS,
	&bitmapMemory,
	0,
	0);


}

internal void
Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
    //rectangle to rectangle copy
    StretchDIBits(deviceContext,
		  x, y, width, height,
		  x, y, width, height,
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
	Win32UpdateWindow(deviceContext, X, Y, WIDTH, HEIGHT);
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
	    MSG message;
	    while(running)
	    {
		BOOL messageResult = GetMessage(&message, 0, 0, 0);
		if (messageResult > 0)
		{
		    TranslateMessage(&message);//a thing that takes a message from the queue and gets it ready to send it out
		    DispatchMessage(&message);
		}
		else
		{
		    break;
		}
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
