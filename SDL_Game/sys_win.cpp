#include <Windows.h>
#include <SDL\SDL.h>

//Linking to our libs
#pragma comment (lib, "SDL2.lib")
#pragma comment (lib, "SDL2main.lib")

bool			isRunning	= false;
SDL_Window*		sdlWindow	= 0;
SDL_Renderer*	renderer	= 0;

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
	wc.lpszClassName = "gj2016";
	wc.hCursor = 0;	//TODO: Add cursors and icons to this program.
	wc.hIcon = 0;
	wc.lpfnWndProc = (WNDPROC)wndProc;

	RegisterClass(&wc);

	HWND window = CreateWindow("gj2016", "SDL Group Project",
								WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT, CW_USEDEFAULT,
								1024, 768,
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
		strcpy(error, SDL_GetError());
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

	//Close
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();
	DestroyWindow(window);

	return 0;
}