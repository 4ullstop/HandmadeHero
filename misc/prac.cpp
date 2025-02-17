#include <windows.h>

#define local_persist static
#define global_variable static
#define internal static

global_variable bool running;
global_variable BITMAPINFO bitmapInfo;
global_variable void* bitmapMemory;
global_variable HBITMAP bitmapHandle;
global_variable HDC bitmapDeviceContext;

internal void
Win32ResizeDIBSection(int width, int height)
{
    if (bitmapHandle)
    {
	DeleteObject(bitmapHandle);
    }

    if (!bitmapDeviceContext)
    {
	bitmapDeviceContext = CreateCompatibleDC(0); //we want to create a compatible dc but not use the one we already have because we will be writing to the buffer before swapping the DC
    }

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = height;
    bitmapInfo.bmiHeader.biPlanes = 1; //number of planes for the target device and must be set to 1
    bitmapInfo.bmiHeader.biBitCount = 32; //num of bits per pixel
    bitmapInfo.bmiHeader.biCompression = BI_RGB; //uncompressed RGB

    bitmapHandle = CreateDIBSection(
	bitmapDeviceContext,
	&bitmapInfo,
	DIB_RGB_COLORS,
	&bitmapMemory,
	0, //a handle to a file mapping object taht the function, null therefore it allocates mem for the DIB
	0); //an offset fro mteh beginning of the file mapping context (if we had one)
}

internal void
Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
    //copies the color data for a rect of pixels
    StretchDIBits(deviceContext,
		  x, y, width, height, //destination info
		  x, y, width, height, //source info
		  bitmapMemory,
		  &bitmapInfo,
		  DIB_RGB_COLORS,
		  SRCCOPY);
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
	GetClientRect(hwnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;
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
	OutputDebugStringA("WM_CLOSE\n"):
    } break;
    case WM_ACTIVATEAPP:
    {
	OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT:
    {
	PAINTSTRUCT paint;
	HDC deviceContext = BeginPaint(hwnd, &paint);

	int height = paint.rcPaintBottom - paint.rcPaint.top;
	int width = paint.rcPaint.right - paint.rcPaint.left;
	int X = paint.rcPaint.left;
	int Y = paint.rcPaint.top;
	Win32UpdateWindow(deviceContext, X, Y, width, height);
	EndPaint(hwnd, &paint);
    } break;
    default:
    {

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
    windowClass.lpfnWndProc = ;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = "Handmade Hero Window Class";

    if (RegisterClass(&windowClass))
    {
	HWND windowHandle = CreateWindowEx(
	    0,
	    windowClass.lpszClassName,
	    "Handmade hero",
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
	    running = true;

	    MSG message;
	    while (running)
	    {
		BOOL messageResult = GetMessage(&message, 0, 0, 0);

		if (messageResult > 0)
		{
		    TranslateMessage(&message);
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
	
}
