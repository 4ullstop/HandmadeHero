#include <windows.h>

LRESULT CALLBACK MainWindowProc(HWND hwnd,
				UINT uMsg,
				WPARAM wParam,
				LPARAM lParam,)
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
	OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE:
    {
	OutputDebugStringA("WM_CLOSE\n");
    } break;
    case WM_ACTIVATEAPP:
    {
	OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT:
    {
	PAINTSTRUCT paint;
	//Prepares the specified window for painting and fills the PAINTSTRUCT w/info abt painting
	HDC deviceContext = BeginPaint(hwnd, &paint);
	int HEIGHT = paint.rcPaint.bottom - paint.rcPaint.top;
	int WIDTH = paint.rcPaint.right - paint.rcPaint.left;
	int x = paint.rcPaint.left;
	int y = paint.rcPaint.top;
	static DWORD operation = WHITENESS;
	if (operation == WHITENESS)
	{
	    operation = BLACKNESS;
	}
	else
	{
	    operation = WHITENESS;
	}
	//paint a specified rectangle using the brush that is currently selected into a specified device context
	PatBlt(deviceContext, x, y, WIDTH, HEIGHT, operation);
	EndPaint(hwnd, &paint);
    } break;
    }
}

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow)
{
    WNDCLASS windowClass = {}; //this contains the attributes registered by the RegisterClass function
    windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    windowClass.lpfnWndProc = ;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = "Handmade hero window class";

    if (RegisterClass(&windowClass))
    {
	HWND windowHandle = CreateWindowEx(
	    WS_EX_CLIENTEDGE, //extended styles
	    windowClass.lpszClassName, //must match the one used above in the windowClass struct
	    "Handmade hero", //optional window name, more useful for windows with controls like buttons
	    WS_OVERLAPPEDWINDOW|WS_VISIBLE, //OverlappedWindow is a default one for basic windows
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
	    MSG message;
	    for (;;)
	    {
		BOOL messageResult = GetMessage(&message, 0, 0, 0);
		if (messageResult > 0)
		{
		    TranslateMessage(&message); //reading the thread's message queue
		    DispatchMessage(&message); //dispatches the read message to the custom window procedure
		}
		else
		{
		    break;
		}
	    }
	}
    }

    return(0);
}
