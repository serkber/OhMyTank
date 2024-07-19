#include <iostream>
#include "windows.h"
#include "OMTGame.h"
#include <Dbt.h>

// Global variables
HINSTANCE g_hInst;	// current instance
HWND g_hWnd;		// main window handle
OMTGame g_game;

HDEVNOTIFY g_hNewAudio = nullptr;

// Function declarations (included in this C++ file)
ATOM				MyRegisterClass(HINSTANCE);
bool				CreateMainWnd(int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


//// Main entry point function
//int APIENTRY wWinMain(HINSTANCE hInstance,
//                      HINSTANCE hPrevInstance,
//                      LPTSTR    lpCmdLine,
//                      int       nCmdShow)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
int main()
{    
HINSTANCE hInstance = GetModuleHandle(NULL);
// Set Subsystem Console

    // Initialization
    g_hInst = hInstance;
    g_hWnd = NULL;
    MyRegisterClass(hInstance);

    // Create main window
    //if (!CreateMainWnd (nCmdShow))
    if (!CreateMainWnd(SW_SHOWNORMAL))
    // Set Subsystem Console
        return -1;

    // Initialize demo
    if (!g_game.Initialize(g_hWnd, g_hInst))
        return -1;

    if(!g_game.LoadContent())
        return -1;

    // Listen for new audio devices
    DEV_BROADCAST_DEVICEINTERFACE filter = {};
    filter.dbcc_size = sizeof( filter );
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = KSCATEGORY_AUDIO;
    g_hNewAudio = RegisterDeviceNotification(g_hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

    // Main message loop
    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        // Update and render
        g_game.PreUpdate();
        g_game.Update();
        g_game.Render();
    }

    // Terminate game
    g_game.Terminate();

    // Release Audio stuff
    if (g_hNewAudio)
    {
        UnregisterDeviceNotification(g_hNewAudio);
        g_hNewAudio = nullptr;
    }
    CoUninitialize();

    return (int)msg.wParam;
}


// Registers the window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= NULL;
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= L"OhMyTank";
    wcex.hIconSm		= NULL;

    return RegisterClassEx(&wcex);
}

// Creates the main window
bool CreateMainWnd(int nCmdShow)
{
    // Calculate main window size
    RECT rc = { 0, 0, 800, 600 };
    ::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the main window
    g_hWnd = ::CreateWindow(L"OhMyTank", L"Oh My Tank!", 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        rc.right - rc.left, rc.bottom - rc.top, 
        NULL, NULL, g_hInst, NULL);
    
    // Check window handle
    if (g_hWnd == NULL)
        return false;

    ::ShowWindow(g_hWnd, nCmdShow);
    ::UpdateWindow(g_hWnd);

    return true;
}


// Processes messages for the main window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
        RECT rc;

    switch (message)
    {
    case WM_DEVICECHANGE:
        if ( wParam == DBT_DEVICEARRIVAL )
        {
            auto pDev = reinterpret_cast<PDEV_BROADCAST_HDR>( lParam );
            if( pDev )
            {
                if ( pDev->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
                {
                    auto pInter = reinterpret_cast<const PDEV_BROADCAST_DEVICEINTERFACE>( pDev );
                    if ( pInter->dbcc_classguid == KSCATEGORY_AUDIO )
                    {
                        g_game.OnNewAudioDevice();
                    }
                }
            }
        }
        return 0;
    case WM_SETFOCUS:
        g_game.SetFocusState(true);
    break;
    case WM_KILLFOCUS:
        g_game.SetFocusState(false);
        break;
    case WM_EXITSIZEMOVE:
        g_game.ReInitializeGraphics(hWnd, g_hInst);
        break;
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 800;
            lpMMI->ptMinTrackSize.y = 600;
        }
    case WM_PAINT:
        hdc = ::BeginPaint(hWnd, &ps);
        ::EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;

    default:
        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
