/*!
  @file
  direct3d_system.h

  @brief
  Functionality for DirectX 11 3D rendering.

  @detail
*/

#pragma once

//--------------------------------------------
// Linking
//--------------------------------------------
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>


namespace Gumshoe {

//--------------------------------------------
// Direct3DSystem class definition
//--------------------------------------------
class Direct3DSystem
{
public:
	Direct3DSystem();
	~Direct3DSystem();

	bool Init(int, int, bool, HWND, bool, float, float);
	void Shutdown();
	
	void BeginScene(float, float, float, float);
	void DrawScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(D3DXMATRIX&);
	void GetWorldMatrix(D3DXMATRIX&);
	void GetOrthoMatrix(D3DXMATRIX&);

	void TurnZBufferOn();
	void TurnZBufferOff();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	void TurnOnCulling();
	void TurnOffCulling();

	void EnableSecondBlendState();

	void SetBackBufferRenderTarget();
	void ResetViewport();

	void GetVideoCardInfo(char*, int&);

private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	
	ID3D11DepthStencilState* m_depthStencilState;         // Used for 3D rendering
	ID3D11DepthStencilState* m_depthDisabledStencilState; // Used for 2D rendering
	ID3D11BlendState* m_alphaEnableBlendingState;         // Used for alpha blending
	ID3D11BlendState* m_alphaDisableBlendingState;        // When alpha blending is disabled
    ID3D11BlendState* m_alphaBlendState2;

	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	ID3D11RasterizerState* m_rasterStateNoCulling;        // Used for sky domes mainly
	D3DXMATRIX m_projectionMatrix;
	D3DXMATRIX m_worldMatrix;
	D3DXMATRIX m_orthoMatrix;
	D3D11_VIEWPORT m_viewport;
};

} // end of namespace Gumshoe