/*!
  @file
  model.h

  @brief
  Functionality needed to display a 3D model.

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

#include <fstream>
using namespace std;

namespace Gumshoe {

//--------------------------------------------
// Model class definition
//--------------------------------------------
class Model
{
private:
	struct modelVertex_t
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	    D3DXVECTOR3 normal;
	};

	struct modelData_t
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	Model();
	~Model();

	bool Init(ID3D11Device*, LPCSTR*, char*, Vector3_t);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	bool UpdatePosition(ID3D11Device*, Vector3_t);

private:
	bool InitBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, LPCSTR*);
	void ReleaseTexture();

	bool LoadModel(char*, Vector3_t);
	void ReleaseModel();

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	modelData_t* m_model;

	Texture* m_Texture;
};

} // end of namespace Gumshoe