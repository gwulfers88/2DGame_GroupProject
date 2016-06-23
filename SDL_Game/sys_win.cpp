#include "sys_win.h"

//DEBUGGIN ONLY
// TODO(George): Make a Hot reload.

// TODO(George): Create File loading systems and File writing systems.
// TODO(George): Create mouse Handling.
// TODO(George): Create Memory handling.

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
    if(WriteFile(state->recordingHandle, input, sizeof(*input), &bytesWritten, 0) == TRUE)
    {

    }
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

            result.isValid = (result.UpdateRender);
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

        r32 dt = 1.0f / RefreshRate;

        u32 totalSize = Megabytes(16);
        gameMemoryBlock = VirtualAlloc( 0, totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if(!gameMemoryBlock)
        {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(sdlWindow);
            SDL_Quit();
            DestroyWindow(window);
            return -5;
        }

        GameController input = {0};
        GameState game = {0};

        game.memoryBlock = gameMemoryBlock;
        game.blockSize = totalSize;
        game.renderer = renderer;
        game.screenW = wndWidth;
        game.screenH = wndHeight;
        game.dt = dt;

        Win32State state = {0};
        state.memoryBlock = gameMemoryBlock;
        state.memorySize = totalSize;

        GameCodeDLL gameCode =  Win32LoadGameCode("Game.dll", "Game_temp.dll", "lock.tmp");
        
        isRunning = true;

        while(isRunning)
        {
            // NOTE: compare File times for us to be able to reload the new game code
            FILETIME currentFileTime = Win32GetLastWriteTime("Game.dll");
            if(CompareFileTime(&currentFileTime, &gameCode.lastWriteTime) != 0)
            {
                Win32UnloadGameCode(&gameCode);
                gameCode = Win32LoadGameCode("Game.dll", "Game_temp.dll", "locktmp");
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
                                input.actionUp.isDown = isDown;
                            }
                            else if(vkCode == 'S')
                            {
                                input.actionDown.isDown = isDown;
                            }
                            else if(vkCode == 'A')
                            {
                                input.actionLeft.isDown = isDown;
                            }
                            else if(vkCode == 'D')
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

            if(gameCode.UpdateRender)
            {
                gameCode.UpdateRender(&game, &input);
            }
        }

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
    SDL_Quit();
    DestroyWindow(window);

    return 0;
}
