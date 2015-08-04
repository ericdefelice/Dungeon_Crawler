/*!
  @file
  debug_window.h

  @brief
  Functionality for the debug window.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <d3d11.h>
#include <d3dx10math.h>

namespace Gumshoe {
	
//--------------------------------------------
// Debug Window class definition
//--------------------------------------------
class DebugWindow
{
private:
	struct vertex_t
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	DebugWindow();
	~DebugWindow();

	bool Init(ID3D11Device*, int, int, int, int);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, int);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};

} // end of namespace Gumshoe
