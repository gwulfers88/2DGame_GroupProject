#include "sys_win.h"

// TODO(George): Create Update method and Rendering methods.
// TODO(George): Create File loading systems and File writing systems.
// TODO(George): Create Keyboard and mouse Handling.
// TODO(George): Create Memory handling.

//Message Handling
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

//WINDOWS MAIN FUNCTION
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
	wc.hCursor = 0;	//TODO: Add cursors and icons to this program.
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

		char* buffer = 0;	//Uninitialized memory
		u32 bufferSize = Kilobytes(1);
		char* buffer2 = 0;	//Uninitialized memory
		u32 buffer2Size = Kilobytes(16);

		buffer = (char*)gameMemoryBlock;
		buffer2 = (char*)gameMemoryBlock + bufferSize;

		stringCopy(buffer, "Writing to my first buffer that was allocated into this block.");
		stringCopy(buffer2, "Now iam writing to buffer2 or something like that that was allocated after buffer 1.");

		isRunning = true;

		while(isRunning)
		{
			MSG msg = {0};
			while(PeekMessage(&msg, window, 0, 0, PM_REMOVE))
			{
				switch(msg.message)
				{
				case WM_CLOSE:
					{
						isRunning = false;
					}break;

				default:
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}

			// TODO: Handle Game Update

			SDL_SetRenderDrawColor(renderer, 50, 150, 250, 255);
			SDL_RenderClear(renderer);

			// TODO: DRAW SHIT HERE

			SDL_RenderPresent(renderer);
		}
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