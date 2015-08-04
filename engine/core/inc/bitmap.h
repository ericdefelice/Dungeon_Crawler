/*!
  @file
  bitmap.h

  @brief
  Functionality needed for displaying 2D bitmaps.

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


namespace Gumshoe {

//--------------------------------------------
// Bitmap class definition
//--------------------------------------------
class Bitmap
{
private:
	struct bitmapVertex_t
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	Bitmap();
	~Bitmap();

	bool Init(ID3D11Device*, int, int, LPCSTR*, int, int);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, int);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, LPCSTR*);
	void ReleaseTexture();

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_prevPosX, m_prevPosY;

	Texture* m_Texture;
};

} // end of namespace Gumshoe