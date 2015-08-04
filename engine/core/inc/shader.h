/*!
  @file
  shader.h

  @brief
  Functionality for the DirectX shaders.

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
// Shader class definition
//--------------------------------------------
class Shader
{
private:
	struct shaderMatrixBuffer_t
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct shaderCameraBuffer_t
	{
		D3DXVECTOR3 cameraPosition;
		float padding;
	};

	struct ambientLightBuffer_t
	{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor;
		D3DXVECTOR3 lightDirection;
	    float       padding;
	};

	struct specularLightBuffer_t
	{
		D3DXVECTOR4 ambientColor;
		D3DXVECTOR4 diffuseColor;
		D3DXVECTOR3 lightDirection;
	    float       specularPower;
	    D3DXVECTOR4 specularColor;
	};

	struct textureInfoBuffer_t
	{
		bool useAlplha;
		D3DXVECTOR3 padding2;
	};

public:
	Shader();
	~Shader();

	bool Init(ID3D11Device*, HWND, LPCSTR*, LPCSTR*, int);
	void Shutdown();
    
    // Functions for rendering different types of shaders
    bool RenderColorShader(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX);
    bool RenderTextureShader(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*);
	bool RenderAmbientShader(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR3, D3DXVECTOR4,
		                     D3DXVECTOR4, D3DXVECTOR3, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*);
	bool RenderSpecularShader(ID3D11DeviceContext*, int, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*, 
		                      D3DXVECTOR3, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3, D3DXVECTOR4, float);

	bool PublicSetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*,
		                           D3DXVECTOR3, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3, D3DXVECTOR4, float, int);
	void PublicRenderShader(ID3D11DeviceContext*, int);
	
    bool SetShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3);
	bool SetShaderTextures(ID3D11DeviceContext*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, bool);

private:
	bool InitShader(ID3D11Device*, HWND, LPCSTR*, LPCSTR*, const LPCSTR*, const LPCSTR*, int);
	bool InitColorShader(ID3D11Device*, HWND, LPCSTR*, LPCSTR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, LPCSTR*);

	// Functions for setting the different shader parameters
	bool SetColorShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX);
	bool SetTextureShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*);
	bool SetAmbientShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR3, D3DXVECTOR4, 
		                            D3DXVECTOR4, D3DXVECTOR3, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*);
	bool SetSpecularShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, ID3D11ShaderResourceView*,
		                             D3DXVECTOR3, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3, D3DXVECTOR4, float);

	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;

	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_textureInfoBuffer;
};

} // end of namespace Gumshoe
