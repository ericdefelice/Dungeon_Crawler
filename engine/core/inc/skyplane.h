/*!
  @file
  skyplane.h

  @brief
  Functionality for creating the plane where the clouds will be rendered on.

  @detail

*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include "texture.h"
using namespace std;


namespace Gumshoe {

//--------------------------------------------
// SkyPlane class definition
//--------------------------------------------
class SkyPlane
{
private:
	struct skyPlane_t
	{
		float x, y, z;
		float tu, tv;
	};

	struct vertex_t
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};

public:
	SkyPlane();
	~SkyPlane();

	bool Init(ID3D11Device*, LPCSTR*, LPCSTR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	void Frame(float);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetCloudTexture();
	ID3D11ShaderResourceView* GetPerturbTexture();
	
	float GetScale();
	float GetBrightness();
	float GetTranslation();

private:
	bool InitializeSkyPlane(int, float, float, float, int);
	void ShutdownSkyPlane();

	bool InitializeBuffers(ID3D11Device*, int);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTextures(ID3D11Device*, LPCSTR*, LPCSTR*);
	void ReleaseTextures();

private:
	skyPlane_t* m_skyPlane;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	Texture *m_CloudTexture, *m_PerturbTexture;
	float m_scale, m_brightness, m_translation;
};

} // end of namespace Gumshoe
