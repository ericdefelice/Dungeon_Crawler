/*!
  @file
  input.h

  @brief
  Engine input handling.

  @detail
  Handles keyboard, mouse, and controller input (using XInput).
*/

#pragma once

//--------------------------------------------
// Defines
//--------------------------------------------
#define DIRECTINPUT_VERSION 0x0800

//--------------------------------------------
// Linking
//--------------------------------------------
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <dinput.h>


namespace Gumshoe {

//--------------------------------------------
// Input class definition
//--------------------------------------------
class Input
{
public:
	Input();
	~Input();

	bool Init(HINSTANCE, HWND, int, int);
    void Shutdown();
	bool Update();

	bool IsEscapePressed();
	bool IsLeftArrowPressed();
	bool IsRightArrowPressed();
	
	bool IsKeyPressed(unsigned char);
	bool IsKeyPressedStrobe(unsigned char);
	void GetMouseLocation(int&, int&);
	void GetMouseMovement(int&, int&);

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	unsigned char m_keyboardState[256];
	unsigned char m_prevKeyboardState[256];
	DIMOUSESTATE m_mouseState;

	int m_screenWidth, m_screenHeight;
	int m_mouseX, m_mouseY, m_mouseDeltaX, m_mouseDeltaY;
};

} // end of namespace Gumshoe