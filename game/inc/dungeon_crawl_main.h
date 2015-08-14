/*!
  @file
  dungeon_crawl_main.h

  @brief
  Top layer for the game.

  @detail
  This will include all the engine components as well as the
  game specific components.  This is just the game wrapper basically.
*/

#pragma once

//--------------------------------------------
// Globals
//--------------------------------------------
const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.01f;

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "input.h"
#include "audio.h"
#include "direct3d_system.h"
#include "camera.h"
#include "shader.h"
#include "timer.h"
#include "fps_count.h"
#include "cpu_load.h"
#include "font_shader.h"
#include "text.h"
#include "light.h"
#include "frustum.h"
#include "entity.h"
/*
#include "debug_window.h"
#include "texture_shader.h"
#include "render_texture.h"
#include "depth_shader.h"
*/
#include "dungeon_world.h"

using namespace Gumshoe;

//--------------------------------------------
// Game class definition
//--------------------------------------------
class Game
{
public:
	Game();
	~Game();

	bool Init(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	bool HandleInput(float);
	bool RenderSceneToTexture();
	bool RenderGraphics();

private:
	// Engine components
	Input* m_Input;
	Audio* m_Audio;
	Direct3DSystem* m_Direct3DSystem;
	Camera* m_Camera;
	Shader* m_Shader;
	Timer* m_Timer;
	FpsCount* m_FpsCount;
	CpuLoad* m_CpuLoad;
	Text* m_Text;
	Light* m_Light;
	Frustum* m_Frustum;
	Entity* m_Player;
/*
	DebugWindow* m_DebugWindow;
	TextureShader* m_TextureShader;
	RenderTexture* m_RenderTexture;
	DepthShader* m_DepthShader;
*/
	// Game specific components
	GameWorld* m_World;
};
