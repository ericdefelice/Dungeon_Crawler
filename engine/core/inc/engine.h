/*!
  @file
  gumshoe_engine.h

  @brief
  Engine platform layer (for Windows now, would need to be ported to other platforms).

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "dungeon_crawl_main.h"


namespace Gumshoe {

//--------------------------------------------
// Engine class definition
//--------------------------------------------
class GumshoeEngine
{
public:
	GumshoeEngine();
	~GumshoeEngine();

	bool Init();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindow(int&, int&, bool);
	void ShutdownWindow(bool);

private:
	LPCTSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	Game* m_Game;
};

} // end of namespace Gumshoe


//--------------------------------------------
// Global Functions
//--------------------------------------------
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//--------------------------------------------
// Global Variables
//--------------------------------------------
static Gumshoe::GumshoeEngine* g_engine = 0;
