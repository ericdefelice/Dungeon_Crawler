/*!
  @file
  dungeon_crawl_main.cpp

  @brief
  Top layer for the game.

  @detail
  This will include all the engine components as well as the
  game specific components.  This is just the game wrapper basically.
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "dungeon_crawl_main.h"

#include "input.cpp"
#include "audio.cpp"
#include "direct3d_system.cpp"
#include "camera.cpp"
#include "shader.cpp"
#include "timer.cpp"
#include "fps_count.cpp"
#include "cpu_load.cpp"
#include "font_shader.cpp"
#include "text.cpp"
#include "light.cpp"
#include "frustum.cpp"
#include "entity.cpp"
/*
#include "debug_window.cpp"
#include "texture_shader.cpp"
#include "render_texture.cpp"
#include "depth_shader.cpp"
*/
#include "dungeon_world.cpp"


Game::Game()
{
	m_Input = nullptr;
	m_Audio = nullptr;
	m_Direct3DSystem = nullptr;
	m_Camera = nullptr;
	m_Shader = nullptr;
	m_Timer = nullptr;
	m_FpsCount = nullptr;
	m_CpuLoad = nullptr;
	m_Text = nullptr;
	m_Frustum = nullptr;
	m_Player = nullptr;
/*
	m_DebugWindow = nullptr;
	m_TextureShader = nullptr;
	m_RenderTexture = nullptr;
	m_DepthShader = nullptr;
*/
	m_World = nullptr;
}


Game::~Game()
{
}


bool Game::Init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result;
	D3DXMATRIX baseViewMatrix;
	//int worldLength, worldWidth;
	char videoCard[128];
	int videoMemory;

	
	//--------------------------------------------
    // Input Initialization
    //--------------------------------------------
	// Create the input object.  The input object will be used to handle reading the keyboard and mouse input from the user.
	m_Input = new Input;
	if(!m_Input)
	{
		return false;
	}

	// Initialize the input object.
	result = m_Input->Init(hinstance, hwnd, screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the input object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


	//--------------------------------------------
    // Audio Initialization
    //--------------------------------------------
	// Create the audio object.
	m_Audio = new Audio;
	if(!m_Audio)
	{
		return false;
	}
 
	// Initialize the sound object.
	result = m_Audio->Init(hwnd);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the audio object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


	//--------------------------------------------
    // Direct3DSystem Initialization
    //--------------------------------------------
	m_Direct3DSystem = new Direct3DSystem;
	if(!m_Direct3DSystem)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_Direct3DSystem->Init(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize DirectX 11."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


	//--------------------------------------------
    // Camera Initialization
    //--------------------------------------------
	m_Camera = new Camera;
	if(!m_Camera)
	{
		return false;
	}

	// Initialize a base view matrix with the camera for 2D user interface rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->RenderBaseViewMatrix();
	m_Camera->GetBaseViewMatrix(baseViewMatrix);

	
    //--------------------------------------------
    // Main Environment Shader Initialization
    //--------------------------------------------
	m_Shader = new Shader;
	if(!m_Shader)
	{
		return false;
	}

    // Set the vertex and pixel shader filenames
    LPCSTR vsFilename = (LPCSTR)"../engine/core/inc/shaders/ambient_light.vs";
	LPCSTR psFilename = (LPCSTR)"../engine/core/inc/shaders/ambient_light.ps";
	
	// Initialize the shader object.
	result = m_Shader->Init(m_Direct3DSystem->GetDevice(), hwnd, &vsFilename, &psFilename, 1);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the main shader object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


	//--------------------------------------------
    // Light Initialization
    //---------------------------------------------
	m_Light = new Light;
	if(!m_Light)
	{
		return false;
	}

	// Initialize the light object to an ambient light
	m_Light->SetAmbientColor(0.50f, 0.50f, 0.50f, 1.0f);
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, -1.0f, 0.0f);

	
	//--------------------------------------------
    // FpsCount Initialization
    //--------------------------------------------
	// Create the fps count object.
	m_FpsCount = new FpsCount;
	if(!m_FpsCount)
	{
		return false;
	}

	// Initialize the fps object.
	m_FpsCount->Init();

	
    //--------------------------------------------
    // CpuLoad Initialization
    //--------------------------------------------
    // Create the cpu load object.
	m_CpuLoad = new CpuLoad;
	if(!m_CpuLoad)
	{
		return false;
	}

	// Initialize the cpu object.
	m_CpuLoad->Init();

	
    //--------------------------------------------
    // Timer Initialization
    //--------------------------------------------
	// Create the timer object.
	m_Timer = new Timer;
	if(!m_Timer)
	{
		return false;
	}

	// Initialize the timer object.
	result = m_Timer->Init();
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the timer object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


    //--------------------------------------------
    // Text Initialization
    //--------------------------------------------
	m_Text = new Text(11);
	if(!m_Text)
	{
		return false;
	}

	// Initialize the text object.
	result = m_Text->Init(m_Direct3DSystem->GetDevice(), m_Direct3DSystem->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the text object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


	// Retrieve the video card information.
	m_Direct3DSystem->GetVideoCardInfo(videoCard, videoMemory);

	// Set the video card information in the text object.
	result = m_Text->SetVideoCardInfo(videoCard, videoMemory, m_Direct3DSystem->GetDeviceContext());
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not set the video card info in the text object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


	//--------------------------------------------
    // Frustum Initialization
    //--------------------------------------------
	// Create the frustum object.
	m_Frustum = new Frustum;
	if(!m_Frustum)
	{
		return false;
	}


	//--------------------------------------------
    // Game World Initialization
    //--------------------------------------------
	// Create the world object.
	m_World = new GameWorld;
	if(!m_World)
	{
		return false;
	}

	// Initialize the game world.
	LPCSTR groundTextureFilename = (LPCSTR)"../assets/textures/dungeon_floor.png";
	LPCSTR wallTextureFilename = (LPCSTR)"../assets/slope.dds";
	
	 // Initialize the game world object.
	result = m_World->Init(m_Direct3DSystem->GetDevice(), &groundTextureFilename, &wallTextureFilename);
	
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the game world."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

/*
    //--------------------------------------------
    // Debug Window Initialization
    //--------------------------------------------
    // Create the debug window bitmap object.
	m_DebugWindow = new DebugWindow;
	if(!m_DebugWindow)
	{
		return false;
	}

	// Initialize the debug window bitmap object.
	result = m_DebugWindow->Init(m_Direct3DSystem->GetDevice(), screenWidth, screenHeight, 100, 100);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the debug window object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	
    //--------------------------------------------
    // Texture Shader Initialization
    //--------------------------------------------
	// Create the texture shader object.
	m_TextureShader = new TextureShader;
	if(!m_TextureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_TextureShader->Init(m_Direct3DSystem->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the texture shader object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	
    //--------------------------------------------
    // Render Texture Initialization
    //--------------------------------------------
	// Create the render to texture object.
	m_RenderTexture = new RenderTexture;
	if(!m_RenderTexture)
	{
		return false;
	}

	// Initialize the render to texture object.
	result = m_RenderTexture->Init(m_Direct3DSystem->GetDevice(), screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the render texture object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	
    //--------------------------------------------
    // Depth Shader Initialization
    //--------------------------------------------
	// Create the depth shader object.
	m_DepthShader = new DepthShader;
	if(!m_DepthShader)
	{
		return false;
	}

	// Initialize the depth shader object.
	result = m_DepthShader->Init(m_Direct3DSystem->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the depth shader object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}
*/

	
	//--------------------------------------------
    // Player Initialization
    //--------------------------------------------
	// Create the player object.
	m_Player = new Entity;
	if(!m_Player)
	{
		return false;
	}

	Vector3_t playerPosition;
	playerPosition.x = 0.0f;
	playerPosition.y = 0.0f;
	playerPosition.z = 0.0f;

	m_World->GetUpStairsLocation(playerPosition.x, playerPosition.z);
	m_Player->SetPosition(playerPosition);

	m_Camera->SetPosition(playerPosition.x+3.0f, playerPosition.y+10.0f, playerPosition.z-3.0f);
	m_Camera->SetRotation(70.0f, -45.0f, 0.0f);


	// Initialize the player model.
	LPCSTR textureFilename = (LPCSTR)"../assets/textures/red.jpg";
	//LPCSTR modelFilename = (LPCSTR)"../assets/cube.gmd";

	Vector3_t playerModelOffset;
	playerModelOffset.x = 0.25f;
	playerModelOffset.y = 0.0f;
	playerModelOffset.z = 0.125f;

	result = m_Player->Init(m_Direct3DSystem->GetDevice(), &textureFilename, "../assets/player_model.gmd", playerModelOffset);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the player."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}


	return true;
}


void Game::Shutdown()
{
	// Release the player entity object.
	if(m_Player)
	{
		delete m_Player;
		m_Player = nullptr;
	}
/*
	// Release the depth shader object.
	if(m_DepthShader)
	{
		m_DepthShader->Shutdown();
		delete m_DepthShader;
		m_DepthShader = nullptr;
	}

	// Release the render to texture object.
	if(m_RenderTexture)
	{
		m_RenderTexture->Shutdown();
		delete m_RenderTexture;
		m_RenderTexture = nullptr;
	}

	// Release the texture shader object.
	if(m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = nullptr;
	}

	// Release the debug window bitmap object.
	if(m_DebugWindow)
	{
		m_DebugWindow->Shutdown();
		delete m_DebugWindow;
		m_DebugWindow = nullptr;
	}
*/

	// Release the game world object.
	if(m_World)
	{
		m_World->Shutdown();
		delete m_World;
		m_World = nullptr;
	}

	// Release the frustum object.
	if(m_Frustum)
	{
		delete m_Frustum;
		m_Frustum = nullptr;
	}

	// Release the text object.
	if(m_Text)
	{
		m_Text->Shutdown();
		delete m_Text;
		m_Text = nullptr;
	}

	// Release the timer object.
	if(m_Timer)
	{
		delete m_Timer;
		m_Timer = nullptr;
	}

	// Release the cpu object.
	if(m_CpuLoad)
	{
		m_CpuLoad->Shutdown();
		delete m_CpuLoad;
		m_CpuLoad = nullptr;
	}

	// Release the light object.
	if(m_Light)
	{
		delete m_Light;
		m_Light = nullptr;
	}

	// Release the fps object.
	if(m_FpsCount)
	{
		delete m_FpsCount;
		m_FpsCount = nullptr;
	}

	// Release the main shader object.
	if(m_Shader)
	{
		m_Shader->Shutdown();
		delete m_Shader;
		m_Shader = nullptr;
	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = nullptr;
	}

	// Release the Direct3D object.
	if(m_Direct3DSystem)
	{
		m_Direct3DSystem->Shutdown();
		delete m_Direct3DSystem;
		m_Direct3DSystem = nullptr;
	}

	// Release the audio object.
	if(m_Audio)
	{
		m_Audio->Shutdown();
		delete m_Audio;
		m_Audio = nullptr;
	}

	// Release the input object.
	if(m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = nullptr;
	}

	return;
}


bool Game::Frame()
{
	bool result;


	// Read the user input.
	result = m_Input->Update();
	if(!result)
	{
		return false;
	}
	
	// Check if the user pressed escape and wants to exit the application.
	if(m_Input->IsEscapePressed() == true)
	{
		return false;
	}

	// Update the system stats.
	m_Timer->Update();
	m_FpsCount->Update();
	m_CpuLoad->Update();

	// Update the FPS value in the text object.
	//result = m_Text->SetString(0, m_Fps->GetFps(), m_Direct3DSystem->GetDeviceContext());
	result = m_Text->SetFps(m_FpsCount->GetFps(), m_Direct3DSystem->GetDeviceContext());
	if(!result)
	{
		return false;
	}
	
	// Update the CPU usage value in the text object.
	//result = m_Text->SetString(1, m_Cpu->GetCpuPercentage(), m_Direct3DSystem->GetDeviceContext());
	result = m_Text->SetCpu(m_CpuLoad->GetCpuPercentage(), m_Direct3DSystem->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	// Do the frame input processing.
	result = HandleInput(m_Timer->GetTime());
	if(!result)
	{
		return false;
	}

	// Render the graphics.
	result = RenderGraphics();
	if(!result)
	{
		return false;
	}

	return result;
}


bool Game::HandleInput(float frameTime)
{
	bool keyDown, result;
	int inputRotX, inputRotY;
	uint32 rotateBitmask = 0; // {Left, Right, Up, Downs}
	uint32 moveBitmask = 0;   // {Forward, Backward, Left, Right, Up, Down}
	                          //  LSB                                 MSB


	// Set the frame time for calculating the updated position.
	// m_Camera->SetFrameTime(frameTime);

	// Handle the input.
	// Use the mouse location for the rotation
	m_Input->GetMouseMovement(inputRotX, inputRotY);
	
	// Check for Left Arrow input for rotate left
	keyDown = m_Input->IsKeyPressed(DIK_LEFT);
	if (keyDown)
	{
		rotateBitmask = rotateBitmask | 0x01;
	}
		
    // Check for Right Arrow input for rotate right
	keyDown = m_Input->IsKeyPressed(DIK_RIGHT);
	if (keyDown)
	{
		rotateBitmask = rotateBitmask | 0x02;
	}

	// Check for Up Arrow input for rotate up
	keyDown = m_Input->IsKeyPressed(DIK_UP);
	if (keyDown)
	{
		rotateBitmask = rotateBitmask | 0x04;
	}

	// Check for Down Arrow input for rotate down
	keyDown = m_Input->IsKeyPressed(DIK_DOWN);
	if (keyDown)
	{
		rotateBitmask = rotateBitmask | 0x08;
	}

    // Check for W Key input for move forward
	keyDown = m_Input->IsKeyPressed(DIK_W);
	if (keyDown)
	{
		moveBitmask = moveBitmask | 0x01;
	}

    // Check for S Key input for move backward
	keyDown = m_Input->IsKeyPressed(DIK_S);
	if (keyDown)
	{
		moveBitmask = moveBitmask | 0x02;
	}

	// Check for A Key input for strafe left
	keyDown = m_Input->IsKeyPressed(DIK_A);
	if (keyDown)
	{
		moveBitmask = moveBitmask | 0x04;
	}

	// Check for D Key input for strafe right
	keyDown = m_Input->IsKeyPressed(DIK_D);
	if (keyDown)
	{
		moveBitmask = moveBitmask | 0x08;
	}

    // Check for PG_UP input for move up
	keyDown = m_Input->IsKeyPressedStrobe(DIK_SPACE);
	if (keyDown)
	{
		moveBitmask = moveBitmask | 0x10;
	}

	// Check for PG_DN input for move down
	keyDown = m_Input->IsKeyPressed(DIK_PGDN);
	if (keyDown)
	{
		moveBitmask = moveBitmask | 0x20;
	}
	
	
    Vector3_t playerPos, playerRot;

    playerRot.x = (float)(inputRotY);
    playerRot.y = (float)(inputRotX);
    playerRot.z = 0.0f;

	// Move and rotate the player based on inputs.
	m_Player->Move(m_Direct3DSystem->GetDevice(), moveBitmask, frameTime);
	m_Player->Rotate(playerRot, frameTime);
    //m_Player->SetRotation(playerRot);

	// Get the position of the player.
	m_Player->GetPosition(playerPos);
	m_Player->GetRotation(playerRot);
/*
	// Get the height of the triangle that is directly underneath the player's position.
	float groundHeight = 0.0f;
	// TODO(ebd): Do collision within the entity.

	//foundHeight =  m_QuadTree->GetHeightAtPosition(playerPos.x, playerPos.z, groundHeight);
	//if(foundHeight && (groundHeight > playerPos.y))
	foundHeight = true;

	if (foundHeight)
	{
		// If there was a triangle found where the player is, then move them so they are on the ground.
		playerPos.y = groundHeight;
		m_Player->SetPosition(playerPos);
	}
*/
/*	
	if (foundHeight && (groundHeight == playerPos.y))
	{
		m_Player->SetOnGround(true);
	}
	else if (foundHeight && (groundHeight < playerPos.y))
	{
		m_Player->SetOnGround(false);
	}
*/
	// Set the position of the camera.
	m_Camera->SetPosition(playerPos.x+3.0f, 10.0f, playerPos.z-3.0f);
	//m_Camera->SetRotation(playerRot.x, playerRot.y, playerRot.z);

	// Update the position values in the text object.
	result = m_Text->SetCameraPosition(playerPos.x, playerPos.y, playerPos.z, m_Direct3DSystem->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	// Update the rotation values in the text object.
	result = m_Text->SetCameraRotation(playerRot.x, playerRot.y, playerRot.z, m_Direct3DSystem->GetDeviceContext());
	if(!result)
	{
		return false;
	}

	// Update the location of the player on the mini map.
	//m_MiniMap->PositionUpdate(playerPos.x, playerPos.z);

	return true;
}


bool Game::RenderSceneToTexture()
{
/*
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Set the render target to be the render to texture.
	m_RenderTexture->SetRenderTarget(m_Direct3DSystem->GetDeviceContext());

	// Clear the render to texture.
	m_RenderTexture->ClearRenderTarget(m_Direct3DSystem->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, projection, ortho, and base view matrices from the camera and Direct3D objects.
	m_Direct3DSystem->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3DSystem->GetProjectionMatrix(projectionMatrix);

	// Render the terrain using the depth shader.
	m_World->Render(m_Direct3DSystem->GetDeviceContext());
	result = m_DepthShader->Render(m_Direct3DSystem->GetDeviceContext(), m_World->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
	if(!result)
	{
		return false;
	}

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3DSystem->SetBackBufferRenderTarget();

	// Reset the viewport back to the original.
	m_Direct3DSystem->ResetViewport();
*/
	return true;
}


bool Game::RenderGraphics()
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, baseViewMatrix;
	D3DXVECTOR3 cameraPosition;
	bool result;

/*
	// First render the scene to a texture.
	result = RenderSceneToTexture();
	if(!result)
	{
		return false;
	}
*/
	// Clear the scene.
	m_Direct3DSystem->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	m_Direct3DSystem->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3DSystem->GetProjectionMatrix(projectionMatrix);
	m_Direct3DSystem->GetOrthoMatrix(orthoMatrix);
	m_Camera->GetBaseViewMatrix(baseViewMatrix);

	// Construct the frustum.
	m_Frustum->ConstructFrustum(SCREEN_DEPTH, projectionMatrix, viewMatrix);

	// Get the position of the camera.
	cameraPosition = m_Camera->GetPosition();

	// Translate the sky dome to be centered around the camera position.
	D3DXMatrixTranslation(&worldMatrix, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	// Reset the world matrix.
	m_Direct3DSystem->GetWorldMatrix(worldMatrix);

	// Render the game world buffers.
	m_World->Render(m_Direct3DSystem->GetDeviceContext());

	// Render the game world buffers using the game world ambient shader.
	//result = m_World->Render(m_Direct3DSystem->GetDeviceContext(), m_TerrainShader, worldMatrix, viewMatrix, projectionMatrix,
	//	                     m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Light->GetDirection());

	

	// Set the game map shader parameters that it will use for rendering.
	//result = m_Shader->PublicSetShaderParameters(m_Direct3DSystem->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
	//				                             m_World->GetTexture(), m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
	//				                             m_Camera->GetPosition(), {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 3);
	
    // Render the game world using the main shader.
	result = m_Shader->RenderAmbientShader(m_Direct3DSystem->GetDeviceContext(), m_World->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
								           m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Camera->GetPosition(),
								           m_World->GetGroundTexture(), m_World->GetWallTexture());
	if(!result)
	{
		return false;
	}

    // Render the player entity.
	m_Player->Render(m_Direct3DSystem->GetDeviceContext());

	// Render the player model using the main shader.
	result = m_Shader->RenderAmbientShader(m_Direct3DSystem->GetDeviceContext(), m_Player->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, 
								           m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(), m_Camera->GetPosition(),
								           m_Player->GetTexture(), m_Player->GetTexture());

	// Render the game world using the quad tree and game map shader.
	//m_QuadTree->Render(m_Frustum, m_Direct3DSystem->GetDeviceContext(), m_Shader);

	// Set the number of rendered game map triangles since some were culled.
	//result = m_Text->SetRenderCount(m_QuadTree->GetDrawCount(), m_Direct3DSystem->GetDeviceContext());
	//if(!result)
	//{
	//	return false;
	//}

    // Turn off the Z buffer to begin all 2D rendering.
	m_Direct3DSystem->TurnZBufferOff();
/*	
	// Put the debug window on the graphics pipeline to prepare it for drawing.
	result = m_DebugWindow->Render(m_Direct3DSystem->GetDeviceContext(), 20, 420);
	// Render the mini map.
	//result = m_MiniMap->Render(m_Direct3DSystem->GetDeviceContext(), worldMatrix, orthoMatrix, m_MiniMapShader);
	if(!result)
	{
		return false;
	}

	// Render the bitmap model using the texture shader and the render to texture resource.
	m_TextureShader->Render(m_Direct3DSystem->GetDeviceContext(), m_DebugWindow->GetIndexCount(), worldMatrix, baseViewMatrix, orthoMatrix, 
							m_RenderTexture->GetShaderResourceView());
*/
	// Turn on the alpha blending before rendering the text.
	m_Direct3DSystem->TurnOnAlphaBlending();

	// Render the text user interface elements.
	result = m_Text->Render(m_Direct3DSystem->GetDeviceContext(), worldMatrix, orthoMatrix);
	if(!result)
	{
		return false;
	}

	// Turn off alpha blending after rendering the text.
	m_Direct3DSystem->TurnOffAlphaBlending();

	// Turn the Z buffer back on.
	m_Direct3DSystem->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_Direct3DSystem->DrawScene();

	return true;
}
