/*!
  @file
  skydome_shader.h

  @brief
  Functionality for the shader responsible for rendering the skydome.

  @detail

*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
using namespace std;


namespace Gumshoe {

//--------------------------------------------
// SkyDomeShader class definition
//--------------------------------------------
class SkyDomeShader
{
private:
	struct matrixBuffer_t
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct gradientBuffer_t
	{
		D3DXVECTOR4 apexColor;
		D3DXVECTOR4 centerColor;
	};

public:
	SkyDomeShader();
	~SkyDomeShader();

	bool Init(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4);

private:
	bool InitializeShader(ID3D11Device*, HWND, LPCSTR*, LPCSTR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, LPCSTR*);

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_gradientBuffer;
};

} // end of namespace Gumshoe
