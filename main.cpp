#include <windows.h>
#include <stdio.h>

bool GlobalRunning;

// ClearBackbuffer(Buffer* buffer, color);
// DisplayBackbuffer(Buffer* buffer, );
// ResizeBackbuffer();


LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam) {
    LRESULT Result = 0;
    
    switch (Message) {
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(Window, &paint);
            
            RECT rc;
            rc.left   = 10;
            rc.top    = 10;
            rc.right  = 200;
            rc.bottom = 100;
            
            // Use wide string + const-correct type
            LPCWSTR text = L"Hello, Win32 DrawText!";
            
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, RGB(100, 100, 255));
            
            DrawTextW(
                      dc,
                      text,
                      -1,
                      &rc,
                      DT_SINGLELINE | DT_CENTER | DT_VCENTER
                      );
            
            EndPaint(Window, &paint);
        }
        break;
        
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}


int CALLBACK
WinMain(_In_ HINSTANCE Instance,
        _In_opt_ HINSTANCE PrevInstance,
        _In_ LPSTR CommandLine,
        _In_ int ShowCode) {
    
    (void)PrevInstance;
    (void)CommandLine;
    (void)ShowCode;
    
    WNDCLASS WindowClass = {0};
    
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //    WindowClass.hIcon;
    WindowClass.lpszClassName = "Breakout";
    
    if (RegisterClassA(&WindowClass)) {
        HWND Window =
            CreateWindowExA(
                            0,
                            WindowClass.lpszClassName,
                            "Breakout",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            0,
                            0,
                            Instance,
                            0);
        if (Window) {
            
            HDC DeviceContext = GetDC(Window);
            
            GlobalRunning = true;
            while (GlobalRunning) {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
                    if (Message.message == WM_QUIT) {
                        GlobalRunning = false;
                    }
                    
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
            }
            
            // render 
            
            
        }
    }
    return (0);
}
