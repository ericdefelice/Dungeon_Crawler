/*!
  @file
  gumshoe_engine.cpp

  @brief
  Engine top layer. Creates all the other components and handles core engine functionality.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "engine.h"
#include "dungeon_crawl_main.cpp"


namespace Gumshoe {

GumshoeEngine::GumshoeEngine()
{
	m_Game = nullptr;
}


GumshoeEngine::~GumshoeEngine()
{
}


bool GumshoeEngine::Init()
{
	int screenWidth, screenHeight;
	bool result;


	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 960;
	screenHeight = 540;

	// Initialize the windows api.
	InitializeWindow(screenWidth, screenHeight, FULL_SCREEN);

	
	//--------------------------------------------
    // Game object Initialization
    //--------------------------------------------
	// Create the game wrapper object.
	m_Game = new Game;
	if(!m_Game)
	{
		return false;
	}

	// Initialize the game wrapper object.
	result = m_Game->Init(m_hinstance, m_hwnd, screenWidth, screenHeight);
	if(!result)
	{
		return false;
	}


	return true;
}


void GumshoeEngine::Shutdown()
{
	// Release the game object.
	if(m_Game)
	{
		m_Game->Shutdown();
		delete m_Game;
		m_Game = nullptr;
	}

	// Shutdown the window.
	ShutdownWindow(FULL_SCREEN);
	
	return;
}


void GumshoeEngine::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));
	
	// Loop until there is a quit message from the window or the user.
	done = false;
	while(!done)
	{
		// Handle the windows messages.
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Frame();
			if(!result)
			{
				done = true;
			}

			// Wait if needed to maintain 60 fps

			// Update timer here, to be used after next frame processing

		}
	}

	return;
}


bool GumshoeEngine::Frame()
{
	bool result;

	// Do the frame processing for the game.
	result = m_Game->Frame();
	if(!result)
	{
		return false;
	}

	return true;
}


LRESULT CALLBACK GumshoeEngine::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}


void GumshoeEngine::InitializeWindow(int& screenWidth, int& screenHeight, bool fullScreenEn)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get an external pointer to this object.	
	g_engine = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = "Gumshoe Engine Game";

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if(fullScreenEn)
	{
		// Determine the resolution of the clients desktop screen.
	    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	    screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	return;
}


void GumshoeEngine::ShutdownWindow(bool fullScreenEn)
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if(fullScreenEn)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to platform.
	g_engine = NULL;

	return;
}

} // end of namespace Gumshoe


LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch(umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);		
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return g_engine->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}
