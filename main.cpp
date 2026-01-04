#include <windows.h>
#include <stdint.h>

#define internal static 
#define local_persist static 
#define global_variable static

#define BRICK_ROWS 5
#define BRICK_COLS 18

#define BRICK_TILE 64
#define BRICK_OFFSET_X 64
#define BRICK_OFFSET_Y 64

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer {
    BITMAPINFO Info;
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct win32_window_dimension {
    int Width;
    int Height;
};

struct player {
    int X;
    int Y;
    int Width;
    int Height;
    uint32 Color;
};

struct ball {
    float X;
    float Y;
    float dX;
    float dY;
    int Width;
    int Height;
    uint32 Color;
    float Speed;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable player GlobalPlayer;
global_variable ball GlobalBall;
global_variable bool VerticalDirection;
global_variable bool HorizontalDirection;

win32_window_dimension Win32GetWindowDimension(HWND Window) {
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    
    return(Result);
}

internal void
DrawRectangle(win32_offscreen_buffer* Buffer, int MinX, int MinY, int MaxX, int MaxY, uint32 Color)
{
    if (MinX < 0) MinX = 0;
    if (MinY < 0) MinY = 0;
    if (MaxX > Buffer->Width) MaxX = Buffer->Width;
    if (MaxY > Buffer->Height) MaxY = Buffer->Height;
    
    uint8* Row = (uint8*)Buffer->Memory + (MinY * Buffer->Pitch) + (MinX * Buffer->BytesPerPixel);
    
    for (int Y = MinY; Y < MaxY; Y += 1) {
        uint32* Pixel = (uint32*)Row;
        
        for (int X = MinX; X < MaxX; X += 1) {
            *Pixel = Color;
            Pixel += 1;
        }
        
        Row += Buffer->Pitch;
    }
}

int grid[12][20] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

int BrickGrid[5][18] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
};

internal void 
DrawBricks(win32_offscreen_buffer* Buffer, int BrickGrid[5][18]) {
    uint32 c1 = 0x0022DDAA;
    uint32 c2 = 0x00FF3311;
    uint32 c3 = 0x00DDDDAA;
    
    int tileSize = 64;
    
    for(int BrickRow = 0; BrickRow < 5; BrickRow += 1) {
        for (int BrickCol = 0; BrickCol < 18; BrickCol += 1) {
            int value = BrickGrid[BrickRow][BrickCol];
            if (value == 0) {
                continue;
            }
            uint32 color;
            if (value == 1) color = c1;
            else if (value == 2) color = c2;
            else if (value == 3) color = c3;
            int padding = 2;
            
            int left   = BrickCol * tileSize + 64 + padding;
            int top    = BrickRow * tileSize + 64 + padding;
            int right  = left + tileSize - padding * 2;
            int bottom = top  + tileSize - padding * 2;
            
            DrawRectangle(Buffer, left, top, right, bottom, color);
        }
    }
}

internal void 
DrawTileMap(win32_offscreen_buffer* Buffer, int grid[12][20]) {
    uint32 c1 = 0x00202020;
    uint32 c2 = 0x00967969;
    uint32 c3 = 0x0022DDAA;
    uint32 c4 = 0x00FF3311;
    uint32 c5 = 0x00DDDDAA;
    
    int tileSize = 64; // this needs to be adjusted more, because it feels too close for me. had to multiply 32 * 2 to get it to get the full length of window
    
    for (int row = 0; row < 12; row += 1) {
        for (int col = 0; col < 20; col += 1) {
            int value = grid[row][col];                     // get the value of the map spot
            
            uint32 color;          
            if (value == 0) color = c1;
            else if (value == 1) color = c2;
            else if (value == 2) color = c3;
            else if (value == 3) color = c4;
            else if (value == 4) color = c5;
            
            int left = col * tileSize;                      // create the coordinates 
            int top = row * tileSize;                            
            int right = left + tileSize;
            int bottom = top + tileSize;                         
            
            DrawRectangle(Buffer, left, top, right, bottom, color); // create the Rectangle
        }
    }
}

internal void 
DrawPlayer(win32_offscreen_buffer* Buffer, player* p) {
    
    int right = p->X + p->Width;
    int bottom = p->Y + p->Height;
    
    DrawRectangle(Buffer, p->X, p->Y, right, bottom, p->Color);
}

internal void 
DrawBall(win32_offscreen_buffer* Buffer, ball* b) {
    
    int right = b->X + b->Width;
    int bottom = b->Y + b->Height;
    
    DrawRectangle(Buffer, b->X, b->Y, right, bottom, b->Color);
}

internal void
Win32DrawTextOverlayBottomLeft(HWND Window, int MarginX, int MarginY, const char* Text, COLORREF Color) {
    HDC DC = GetDC(Window);
    
    SetBkMode(DC, TRANSPARENT);
    SetTextColor(DC, Color);
    
    RECT Client;
    GetClientRect(Window, &Client);
    
    SIZE TextSize;
    GetTextExtentPoint32A(DC, Text, lstrlenA(Text), &TextSize);
    
    int X = MarginX;
    int Y = (Client.bottom - MarginY) - TextSize.cy;
    if (Y < 0) Y = 0;
    
    TextOutA(DC, X, Y, Text, lstrlenA(Text));
    
    ReleaseDC(Window, DC);
}

internal bool
PointInRect(float px, float py, int left, int top, int right, int bottom)
{
    return (px >= (float)left && px < (float)right &&
            py >= (float)top  && py < (float)bottom);
}

internal void
BallVsBricks(void)
{
    // Ball bounds
    float ballLeft   = GlobalBall.X;
    float ballRight  = GlobalBall.X + (float)GlobalBall.Width;
    float ballTop    = GlobalBall.Y;
    float ballBottom = GlobalBall.Y + (float)GlobalBall.Height;
    
    // We will test a few sample points on the ball.
    // (This avoids scanning all bricks.)
    float testX[2] = { ballLeft, ballRight };
    float testY[2] = { ballTop,  ballBottom };
    
    for (int yi = 0; yi < 2; yi += 1)
    {
        for (int xi = 0; xi < 2; xi += 1)
        {
            float px = testX[xi];
            float py = testY[yi];
            
            // Convert pixel -> brick cell
            int col = (int)((px - BRICK_OFFSET_X) / (float)BRICK_TILE);
            int row = (int)((py - BRICK_OFFSET_Y) / (float)BRICK_TILE);
            
            if (row < 0 || row >= BRICK_ROWS) continue;
            if (col < 0 || col >= BRICK_COLS) continue;
            
            if (BrickGrid[row][col] == 0) continue; // empty cell
            
            // Compute the brick rect (MUST match your DrawBricks math)
            int left   = col * BRICK_TILE + BRICK_OFFSET_X;
            int top    = row * BRICK_TILE + BRICK_OFFSET_Y;
            int right  = left + BRICK_TILE;
            int bottom = top  + BRICK_TILE;
            
            // Confirm overlap (optional but good)
            bool overlap =
            (ballRight  > (float)left) &&
            (ballLeft   < (float)right) &&
            (ballBottom > (float)top) &&
            (ballTop    < (float)bottom);
            
            if (overlap)
            {
                // Remove / damage brick
                BrickGrid[row][col] = 0;
                
                // Reflect ball (simple version: flip vertical)
                VerticalDirection = !VerticalDirection;
                
                // Push ball out a bit so it doesn't "stick"
                if (VerticalDirection == 0) { GlobalBall.Y = (float)top - (float)GlobalBall.Height; }
                else                        { GlobalBall.Y = (float)bottom; }
                
                return; // only one brick per frame
            }
        }
    }
}


internal void 
UpdateBall() {
    
    float PrevX = GlobalBall.X;
    float PrevY = GlobalBall.Y;
    
    // up & left
    if (VerticalDirection == 0 && HorizontalDirection == 0){
        GlobalBall.Y--;
        GlobalBall.X--;
    }
    
    // down & right 
    if (VerticalDirection == 1 && HorizontalDirection == 1){
        GlobalBall.Y++;
        GlobalBall.X++;
    }
    // up & right
    if (VerticalDirection == 0 && HorizontalDirection == 1) {
        GlobalBall.Y--;
        GlobalBall.X++;
    }
    
    // down & left
    if (VerticalDirection == 1 && HorizontalDirection == 0) {
        GlobalBall.Y++;
        GlobalBall.X--;
    }
    
    float MinX = 64.0f;
    float MaxX = 1184.0f;
    float MinY = 64.0f;
    float MaxY = 672.0f;
    
    // Left wall
    if (GlobalBall.X <= MinX) {
        GlobalBall.X = MinX;          // correct position
        HorizontalDirection = 1;      // go right
    }
    
    // Right wall
    if (GlobalBall.X >= MaxX) {
        GlobalBall.X = MaxX;          // correct position
        HorizontalDirection = 0;      // go left
    }
    
    // Top wall
    if (GlobalBall.Y <= MinY) {
        GlobalBall.Y = MinY;          // correct position
        VerticalDirection = 1;        // go down
    }
    
    // Bottom wall
    if (GlobalBall.Y >= MaxY) {
        GlobalBall.Y = MaxY;          // correct position
        VerticalDirection = 0;        // go up
    }
    
    // ball rect (current)
    float BallMinX = GlobalBall.X;
    float BallMaxX = GlobalBall.X + GlobalBall.Width;
    float BallMinY = GlobalBall.Y;
    float BallMaxY = GlobalBall.Y + GlobalBall.Height;
    
    // ball rect (previous)
    float PrevBallMaxY = PrevY + GlobalBall.Height;
    
    // player rect
    float PlayerMinX = (float)GlobalPlayer.X;
    float PlayerMaxX = PlayerMinX + GlobalPlayer.Width;
    float PlayerMinY = (float)GlobalPlayer.Y;
    
    bool OverlapX =
    (BallMaxX > PlayerMinX) &&
    (BallMinX < PlayerMaxX);
    
    bool CrossedTop =
    (PrevBallMaxY <= PlayerMinY) &&
    (BallMaxY >= PlayerMinY);
    
    if (OverlapX && CrossedTop && VerticalDirection == 1) {
        VerticalDirection = 0;
        GlobalBall.Y = PlayerMinY - GlobalBall.Height;
    }
    
    BallVsBricks();
    
    
}

internal void 
ProcessPlayerInput() {
    bool LeftKeyIsDown;
    bool RightKeyIsDown;
    bool LeftKeyWasDown;
    bool RightKeyWasDown;
    
    LeftKeyIsDown = (GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000);
    RightKeyIsDown = (GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000);
    
    if (LeftKeyIsDown) {
        if (GlobalPlayer.X > 64) GlobalPlayer.X--;
    }
    if (RightKeyIsDown) {
        if (GlobalPlayer.X < 1152) GlobalPlayer.X++;
    }
    
    LeftKeyWasDown = LeftKeyIsDown;
    RightKeyWasDown = RightKeyIsDown;
    
}

internal void
ClearBackbuffer(win32_offscreen_buffer* Buffer, uint32 Color) {
    DrawRectangle(Buffer, 0, 0, Buffer->Width, Buffer->Height, Color);
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height) {
    
    if (Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    
    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;
    
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width * BytesPerPixel;
    
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
                           int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer Buffer) {
    // TODO): Aspect ratio correction
    // TODO): Play with stretch modes
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer.Width, Buffer.Height,
                  Buffer.Memory,
                  &Buffer.Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam) {
    LRESULT Result = 0;
    
    switch (Message) {
        case WM_CLOSE:
        {
            // TODO: Handle this with a message to the user?
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_DESTROY:
        {
            // TODO: Handle this as an error - recreate window?
            GlobalRunning = false;
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(
                                       DeviceContext,
                                       Dimension.Width,
                                       Dimension.Height,
                                       GlobalBackbuffer
                                       );
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            // OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode) {
    WNDCLASS WindowClass = {};
    
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 768); 
    
    // TODO: Check if HREDRAW/VREDRAW/OWNDC still matter
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //    WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    
    if (RegisterClassA(&WindowClass)) {
        HWND Window =
            CreateWindowExA(
                            0,
                            WindowClass.lpszClassName,
                            "Handmade Hero",
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
            
            GlobalPlayer.X = 600;
            GlobalPlayer.Y = 600;
            GlobalPlayer.Width = 64;
            GlobalPlayer.Height = 64;
            GlobalPlayer.Color = 0x00FF69B4;
            
            GlobalBall.X = 450;
            GlobalBall.Y = 550;
            GlobalBall.dX = 2;
            GlobalBall.dY = 2;
            GlobalBall.Width = 32.0;
            GlobalBall.Height = 32.0;
            GlobalBall.Color = 0x00114455;
            GlobalBall.Speed = 400.0;
            
            VerticalDirection = 0; 
            HorizontalDirection = 0;
            
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
                
                // ProcessInput
                ProcessPlayerInput();
                UpdateBall();
                
                // Game Rendering
                ClearBackbuffer(&GlobalBackbuffer, 0x00202020);
                DrawTileMap(&GlobalBackbuffer, grid);
                DrawBricks(&GlobalBackbuffer, BrickGrid);
                DrawPlayer(&GlobalBackbuffer, &GlobalPlayer);
                DrawBall(&GlobalBackbuffer, &GlobalBall);
                
                HDC DeviceContext = GetDC(Window);
                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackbuffer);
                ReleaseDC(Window, DeviceContext);
                
                // HUD
                Win32DrawTextOverlayBottomLeft(Window, 20, 20, "START MENU", RGB(255, 255, 0));
                
            }
        }
        else {
            // TODO: Logging
        }
    }
    else {
        // TODO: Logging
    }
    
    return(0);
}