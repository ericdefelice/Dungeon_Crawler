/*!
  @file
  font_shader.h

  @brief
  Functionality for the DirectX shaders for font rendering.

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


namespace Gumshoe {

//--------------------------------------------
// FontShader class definition
//--------------------------------------------
class FontShader
{
private:
	struct shaderMatrixBuffer_t
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct shaderPixelBuffer_t
	{
		D3DXVECTOR4 pixelColor;
	};

public:
	FontShader();
	~FontShader();

	bool Init(ID3D11Device*, HWND, LPCSTR*, LPCSTR*);
	void Shutdown();
    bool Render(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, D3DXVECTOR4);

private:
	bool InitShader(ID3D11Device*, HWND, LPCSTR*, LPCSTR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, LPCSTR*);

	bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, D3DXVECTOR4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;

	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_pixelBuffer;
};

} // end of namespace Gumshoe
