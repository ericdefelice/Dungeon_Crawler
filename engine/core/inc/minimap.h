/*!
  @file
  minimap.h

  @brief
  Functionality for the game minimap.

  @detail

*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "bitmap.h"
#include "shader.h"


namespace Gumshoe {

//--------------------------------------------
// MiniMap class definition
//--------------------------------------------
class MiniMap
{
public:
	MiniMap();
	~MiniMap();

	bool Init(ID3D11Device*, HWND, int, int, D3DXMATRIX, float, float);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, Shader*);
	
	void PositionUpdate(float, float);

private:
	int m_mapLocationX, m_mapLocationY, m_pointLocationX, m_pointLocationY;
	float m_mapSizeX, m_mapSizeY, m_worldLength, m_worldWidth;
	D3DXMATRIX m_viewMatrix;
	Bitmap *m_MiniMapBitmap, *m_Border, *m_Point;
};

} // end of namespace Gumshoe