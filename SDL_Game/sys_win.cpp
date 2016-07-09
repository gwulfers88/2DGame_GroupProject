#include "sys_win.h"
#include <stdio.h>

/*
  TODO(George): THIS IS A SIMPLE PLATFORM LAYER
  - WM_ACTIVATEAPP (for when we are not the active application)
  - control cursor visibility (WM_SETCURSOR)
  - Saved Game Locations (Not in game saving).
  - Getting a handle to our own exe.
  - Asset loading path
  - Multi-Threading (launch a thread)
  - Sleep/timeBeginPeriod
  - Create mouse Handling.
  - Create Memory handling.
  - ClipCursor (for multimonitor support).
  
  NOTE: This is a partial list of what needs to get done
*/

static i64 globalPerformanceCountFreq = 0;
WINDOWPLACEMENT globalWindowPos = { sizeof(globalWindowPos) };
bool isFullscreen = false;

// Thank you Raymond Chen!!
void Win32FullscreenToggle(HWND window)
{
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        if (GetWindowPlacement(window, &globalWindowPos) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {//NOT BITWISE OPERATOR
            SetWindowLong(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &globalWindowPos);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

Win32Dims GetWindowDimensions(HWND window)
{
    RECT clientArea;
    GetClientRect(window, &clientArea);
    Win32Dims result = {};
    result.width = clientArea.right - clientArea.left;
    result.height = clientArea.bottom - clientArea.top;
    return result;
}

void Win32UpdateWindow(Win32Dims windowDims, i32* oldWidth, i32* oldHeight)
{
    if(isFullscreen)
    {
        *oldWidth = windowDims.width;
        *oldHeight = windowDims.height;
    }
    else
    {
        *oldWidth = wndWidth;
        *oldHeight = wndHeight;
    }
}

// Message Handling
LRESULT CALLBACK wndProc(HWND hwnd, Uint32 msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    switch(msg)
    {
        case WM_CLOSE:
        {
            isRunning = false;
            PostQuitMessage(0);
        }break;

        default:
        {
            result = DefWindowProc(hwnd, msg, wparam, lparam);
        }
    }

    return result;
}

void BeginRecording(Win32State* state, u32 index)
{
    state->recordingHandle = CreateFile("recording.wrf", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    state->recordingIndex = index;

    DWORD bytesToWrite = state->memorySize;
    DWORD bytesWritten = 0;
    WriteFile(state->recordingHandle, state->memoryBlock, bytesToWrite, &bytesWritten, 0);
}

void RecordingInput(Win32State* state, GameController* input)
{
    DWORD bytesWritten = 0;
    WriteFile(state->recordingHandle, input, sizeof(*input), &bytesWritten, 0);
}

void EndRecording(Win32State* state)
{
    CloseHandle(state->recordingHandle);
    state->recordingIndex = 0;
}

void BeginPlayback(Win32State* state, u32 index)
{
    state->playbackHandle = CreateFile("recording.wrf", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    state->playbackIndex = index;

    DWORD bytesToRead = state->memorySize;
    DWORD bytesRead = 0;
    ReadFile(state->playbackHandle, state->memoryBlock, bytesToRead, &bytesRead, 0);
}

void EndPlayback(Win32State* state)
{
    CloseHandle(state->playbackHandle);
    state->playbackIndex = 0;
}

void PlaybackInput(Win32State* state, GameController* input)
{
    DWORD bytesRead = 0;

    if(ReadFile(state->playbackHandle, input, sizeof(*input), &bytesRead, 0))
    {
        if(bytesRead == 0)
        {
            u32 playbackIndex = state->playbackIndex;
            EndPlayback(state);
            BeginPlayback(state, playbackIndex);
            ReadFile(state->playbackHandle, input, sizeof(*input), &bytesRead, 0);
        }
    }
}

FILETIME Win32GetLastWriteTime(char* sourceNameDLL)
{
    FILETIME result = {0};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if(GetFileAttributesEx(sourceNameDLL, GetFileExInfoStandard, &data))
    {
        result = data.ftLastWriteTime;
    }

    return result;
}

GameCodeDLL Win32LoadGameCode(char* sourceNameDLL, char* tempNameDLL, char* lockFileName)
{
    GameCodeDLL result = {};

    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if(!GetFileAttributesEx(lockFileName, GetFileExInfoStandard, &ignored))
    {
        result.lastWriteTime = Win32GetLastWriteTime(sourceNameDLL);

        CopyFile(sourceNameDLL, tempNameDLL, FALSE);

        result.codeLib = LoadLibrary(tempNameDLL);
        if(result.codeLib)
        {
            result.UpdateRender = (update_render*)GetProcAddress(result.codeLib, "UpdateRender");

            result.isValid = ((result.UpdateRender) ? true : false);
        }
    }

    if(!result.isValid)
    {
        result.UpdateRender = 0;
    }

    return result;
}

void Win32UnloadGameCode(GameCodeDLL* gameCode)
{
    if(gameCode->codeLib)
    {
        FreeLibrary(gameCode->codeLib);
        gameCode->codeLib = 0;
    }

    gameCode->isValid = false;
    gameCode->UpdateRender = 0;
}

LARGE_INTEGER Win32GetClock(void)
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

r32 Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    r32 result = ((r32)(end.QuadPart - start.QuadPart) /
                  (r32)globalPerformanceCountFreq);
    return result;
}

FREE_FILE(FreeFile)
{
    if(fileResult->data)
    {
        VirtualFree(fileResult->data, 0, MEM_RELEASE);
        fileResult->data = 0;
    }
}

READ_ENTIRE_FILE(ReadEntireFile)
{
    ReadFileResult result = {0};

    HANDLE fileHandle = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if(GetFileSizeEx(fileHandle, &fileSize))
        {
            u32 fileSize32 = (u32)fileSize.QuadPart;
            result.data = VirtualAlloc(0, fileSize32,
                                      MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if(result.data)
            {
                DWORD bytesRead = 0;
                if(ReadFile(fileHandle, result.data, fileSize32, &bytesRead, 0) &&
                   (fileSize32 == bytesRead))
                {
                    //NOTE: file read successfully
                    result.fileSize = bytesRead;
                }
                else
                {
                    //TODO: Logging
                    FreeFile(&result);
                }
            }
            else
            {
                //TODO: Logging
            }
        }
        else
        {
            //TODO: Logging
        }

        CloseHandle(fileHandle);
    }
    else
    {
        //TODO: Logging
    }

    return result;
}

WRITE_ENTIRE_FILE(WriteEntireFile)
{
    HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if(fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesToWrite = 0;
        WriteFile(fileHandle, fileResult->data, fileResult->fileSize, &bytesToWrite, 0);

        CloseHandle(fileHandle);
    }
    else
    {
        //TODO: Logging
    }
}

// WINDOWS MAIN FUNCTION
// hInst = current instance of the program
// hPrevInst = previous instance which is not used anymore.
// cmdLine = holds command line arguments to be passed in to the program
// cmdShow = holds an integer to specify if we want to show this window.
int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow)
{
    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = wndClassName;
    wc.hCursor = 0; //TODO: Add cursors and icons to this program.
    wc.hIcon = 0;
    wc.lpfnWndProc = (WNDPROC)wndProc;

    RegisterClass(&wc);

    HWND window = CreateWindow(wndClassName, wndTitle,
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               wndWidth, wndHeight,
                               0, 0, hInst, 0);
    
    if(window)
    {
        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);

        // NOTE: Initializing SDL
        if(SDL_Init(SDL_INIT_VIDEO) != 0 )
        {
            DestroyWindow(window);
            return -2;
        }

        IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
        
        sdlWindow = SDL_CreateWindowFrom((void*)window);

        char error[MAX_PATH];
        stringCopy(error, SDL_GetError());
        OutputDebugStringA(error);

        if(!sdlWindow)
        {
            SDL_Quit();
            DestroyWindow(window);
            return -3;
        }
        
        renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if(!renderer)
        {
            SDL_DestroyWindow(sdlWindow);
            SDL_Quit();
            DestroyWindow(window);
            return -4;
        }

        i32 RefreshRate = 0;
        HDC dc = GetDC(window);
        i32 winRefreshRate = GetDeviceCaps(dc, VREFRESH);

        if( winRefreshRate > 1 )
        {
            RefreshRate = winRefreshRate / 2;
        }
        else
        {
            RefreshRate = 30;
        }

        r32 targetSecsPerFrame = 1.0f / RefreshRate;

        GameMemory memory = {};
        memory.permanentSize = Megabytes(64);
        memory.transientSize = Megabytes(64);
        memory.totalSize = memory.permanentSize + memory.transientSize;
        
        gameMemoryBlock = VirtualAlloc( 0, memory.totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if(!gameMemoryBlock)
        {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(sdlWindow);
            SDL_Quit();
            DestroyWindow(window);
            return -5;
        }

        memory.permanentBlock = gameMemoryBlock;
        memory.transientBlock = (i8*)gameMemoryBlock + memory.permanentSize;
        memory.readEntireFile = ReadEntireFile;
        memory.freeFile = FreeFile;
        memory.writeEntireFile = WriteEntireFile;

        Win32Dims windowDims = GetWindowDimensions(window);
        
        Render render = {};
        render.renderer = renderer;
        render.screenW = windowDims.width;
        render.screenH = windowDims.height;
        
        GameController input = {0};
        input.dt = targetSecsPerFrame;
        
        Win32State state = {0};
        state.memoryBlock = gameMemoryBlock;
        state.memorySize = memory.totalSize;

        GameCodeDLL gameCode =  Win32LoadGameCode("Game.dll", "Game_temp.dll", "lock.tmp");
        
        isRunning = true;

        r32 sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
        
        LARGE_INTEGER lastCounter = Win32GetClock();

        i64 lastCycles = __rdtsc();
        
        LARGE_INTEGER performanceFreqPerSecRes;
        QueryPerformanceFrequency(&performanceFreqPerSecRes);
        globalPerformanceCountFreq = performanceFreqPerSecRes.QuadPart;
        
        // NOTE: PROGRAM LOOP!!
        while(isRunning)
        {
            // NOTE: compare File times for us to be able to reload the new game code
            FILETIME currentFileTime = Win32GetLastWriteTime("Game.dll");
            if(CompareFileTime(&currentFileTime, &gameCode.lastWriteTime) != 0)
            {
                Win32UnloadGameCode(&gameCode);
                gameCode = Win32LoadGameCode("Game.dll", "Game_temp.dll", "lock.tmp");
            }
            
            MSG msg = {0};
            while(PeekMessage(&msg, window, 0, 0, PM_REMOVE))
            {
                switch(msg.message)
                {
                    case WM_CLOSE:
                    {
                        isRunning = false;
                    }break;

                    case WM_KEYUP:
                    case WM_KEYDOWN:
                    case WM_SYSKEYUP:
                    case WM_SYSKEYDOWN:
                    {
                        u32 vkCode = (u32)msg.wParam;   // This contains the keycode for the key that the user pressed.
                        bool isDown = ((msg.lParam & (1 << 31)) == 0);  // Check to see if the key is down now.
                        bool wasDown = ((msg.lParam & (1 << 30)) != 0); // Check to see if the key was down previously.

                        if(isDown != wasDown)
                        {
                            if(vkCode == 'W')
                            {
                                input.moveUp.isDown = isDown;
                            }
                            else if(vkCode == 'S')
                            {
                                input.moveDown.isDown = isDown;
                            }
                            else if(vkCode == 'A')
                            {
                                input.moveLeft.isDown = isDown;
                            }
                            else if(vkCode == 'D')
                            {
                                input.moveRight.isDown = isDown;
                            }
                            if(vkCode == VK_UP)
                            {
                                input.actionUp.isDown = isDown;
                            }
                            else if(vkCode == VK_DOWN)
                            {
                                input.actionDown.isDown = isDown;
                            }
                            else if(vkCode == VK_LEFT)
                            {
                                input.actionLeft.isDown = isDown;
                            }
                            else if(vkCode == VK_RIGHT)
                            {
                                input.actionRight.isDown = isDown;
                            }
                            else if(vkCode == VK_ESCAPE)
                            {
                                input.back.isDown = isDown;
                            }
                            else if(vkCode == 'O')
                            {
                                if(isDown)
                                {
                                    if(state.recordingIndex == 0)
                                    {
                                        BeginRecording(&state, 1);
                                    }
                                    else
                                    {
                                        EndRecording(&state);
                                        BeginPlayback(&state, 1);
                                    }
                                }
                            }
                            else if(vkCode == 'P')
                            {
                                if(isDown)
                                {
                                    if(state.playbackIndex > 0)
                                    {
                                        EndPlayback(&state);
                                        ZeroMemory(&input, sizeof(input));
                                    }
                                }
                            }
                        }

                        if(isDown)
                        {
                            bool AltKeyWasDown = ((msg.lParam & (1 << 29)) != 0);
                            if(vkCode == VK_RETURN && AltKeyWasDown)
                            {
                                if(msg.hwnd)
                                {
                                    isFullscreen = !isFullscreen;
                                    Win32FullscreenToggle(msg.hwnd);
                                }
                            }
                        }

                    }break;

                    default:
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }

            if(input.back.isDown)
            {
                isRunning = false;
                PostQuitMessage(0);
            }

            if(state.recordingIndex > 0)
            {
                RecordingInput(&state, &input);
            }
            else if(state.playbackIndex > 0)
            {
                PlaybackInput(&state, &input);
            }

            Win32Dims windowDims = GetWindowDimensions(window);
            Win32UpdateWindow(windowDims, (i32*)&render.screenW, (i32*)&render.screenH);
            
            if(gameCode.UpdateRender)
            {
                gameCode.UpdateRender(&memory, &input, &render);
            }

            LARGE_INTEGER workCounter = Win32GetClock();
            r32 workSecsElapsed = Win32GetSecondsElapsed(lastCounter, workCounter);

            r32 secondsElapsed = workSecsElapsed;
            if(secondsElapsed < targetSecsPerFrame)
            {
                if(sleepIsGranular)
                {
                    DWORD sleepMS = (DWORD)(1000.0f * (targetSecsPerFrame - secondsElapsed));
                    if(sleepMS > 0)
                    {
                        Sleep(sleepMS);
                    }
                }

                r32 testSecsElapsed = Win32GetSecondsElapsed(lastCounter, Win32GetClock());
                if(testSecsElapsed < targetSecsPerFrame)
                {
                    //TODO: LOG MISSED SLEEP HERE!!
                }

                while(secondsElapsed < targetSecsPerFrame)
                {
                    secondsElapsed = Win32GetSecondsElapsed(lastCounter, Win32GetClock());
                }
            }
            else
            {
                //TODO: MISSED FRAME RATE!!
            }
            
            LARGE_INTEGER endCounter = Win32GetClock();

            i64 endCycles = __rdtsc();
            
            r64 elapsedCounts = (r64)(endCounter.QuadPart - lastCounter.QuadPart);
            r64 elapsedCycles = (r64)(endCycles - lastCycles);

            r32 MSperFrame = ((1000.0f * Win32GetSecondsElapsed(lastCounter, endCounter)));
            r32 FPS = (r32)(globalPerformanceCountFreq / elapsedCounts);
            r32 MegaCyclesPerFrame = (r32)(elapsedCycles / (1000.0f * 1000.0f));
            
            char buffer[256];
            sprintf_s(buffer, "%.02fms, %.02ffps, %.02fmcpf\n", MSperFrame, FPS, MegaCyclesPerFrame);            
            OutputDebugStringA(buffer);
            
            lastCounter = endCounter;
            lastCycles = endCycles;
            
        } // NOTE: END OF WHILE LOOP

        //IMPORTANT: Unload this when we exit the program.
        Win32UnloadGameCode(&gameCode);        
    }
    else
    {
        // TODO: Handle Error Loggin here!!
        return -1;
    }

    if(gameMemoryBlock)
    {
        VirtualFree(gameMemoryBlock, 0, MEM_RELEASE);
    }
    
    //Close
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(sdlWindow);
    IMG_Quit();
    SDL_Quit();
    DestroyWindow(window);

    return 0;
}
