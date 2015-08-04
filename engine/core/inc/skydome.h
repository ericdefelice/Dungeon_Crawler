/*!
  @file
  skydome.h

  @brief
  Functionality for sky domes in the engine.

  @detail

*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <fstream>
using namespace std;


namespace Gumshoe {

//--------------------------------------------
// SkyDome class definition
//--------------------------------------------
class SkyDome
{
private:
	struct vertex_t
	{
		D3DXVECTOR3 position;
	};

	struct model_t
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	SkyDome();
	~SkyDome();

	bool Init(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	D3DXVECTOR4 GetApexColor();
	D3DXVECTOR4 GetCenterColor();

private:
	bool LoadSkyDomeModel(char*);
	void ReleaseSkyDomeModel();

	bool InitializeBuffers(ID3D11Device*);
	void ReleaseBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	model_t* m_model;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	D3DXVECTOR4 m_apexColor, m_centerColor;
};

} // end of namespace Gumshoe
