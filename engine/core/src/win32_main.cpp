/*!
  @file
  win32_main.cpp

  @brief
  Main entry point for Windows platforms.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include <windows.h>
#include "engine.cpp"


//--------------------------------------------
// Main Windows entry point
//--------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	Gumshoe::GumshoeEngine* gameEngine;
	bool result;
	
	// Create the game engine object.
	gameEngine = new Gumshoe::GumshoeEngine;
	if(!gameEngine)
	{
		return 0;
	}

	// Initialize and run the game engine object.
	result = gameEngine->Init();
	if(result)
	{
		gameEngine->Run();
	}

	// Shutdown and release the game engine object.
	gameEngine->Shutdown();
	delete gameEngine;
	gameEngine = nullptr;

	return 0;
}