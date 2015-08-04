/*!
  @file
  minimap.cpp

  @brief
  Functionality for the game minimap.

  @detail

*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "minimap.h"
#include "bitmap.cpp"


namespace Gumshoe {

MiniMap::MiniMap()
{
	m_MiniMapBitmap = nullptr;
	m_Border = nullptr;
	m_Point = nullptr;
}


MiniMap::~MiniMap()
{
}


bool MiniMap::Init(ID3D11Device* device, HWND hwnd, int screenWidth, int screenHeight, D3DXMATRIX viewMatrix, float worldLength, 
				   float worldWidth)
{
	bool result;


	// Initialize the location of the mini-map on the screen.
	m_mapLocationX = 20;
	m_mapLocationY = 420;

	// Set the size of the mini-map.
	m_mapSizeX = 100.0f;
	m_mapSizeY = 100.0f;

	// Store the base view matrix.
	m_viewMatrix = viewMatrix;

	// Store the world size.
	m_worldLength = worldLength;
	m_worldWidth = worldWidth;

	// Create the mini-map bitmap object.
	m_MiniMapBitmap = new Bitmap;
	if(!m_MiniMapBitmap)
	{
		return false;
	}

	// Initialize the mini-map bitmap object.
	LPCSTR mapFilename = (LPCSTR)"../assets/colormap01.dds";
	result = m_MiniMapBitmap->Init(device, screenWidth, screenHeight, &mapFilename, 100, 100);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the mini-map."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	// Create the border bitmap object.
	m_Border = new Bitmap;
	if(!m_Border)
	{
		return false;
	}

	// Initialize the border bitmap object.
	mapFilename = (LPCSTR)"../assets/border01.dds";
	result = m_Border->Init(device, screenWidth, screenHeight, &mapFilename, 104, 104);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the mini-map border."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	// Create the point bitmap object.
	m_Point = new Bitmap;
	if(!m_Point)
	{
		return false;
	}

	// Initialize the point bitmap object.
	mapFilename = (LPCSTR)"../assets/point01.dds";
	result = m_Point->Init(device, screenWidth, screenHeight, &mapFilename, 3, 3);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the mini-map player pointer."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	return true;
}


void MiniMap::Shutdown()
{
	// Release the point bitmap object.
	if(m_Point)
	{
		m_Point->Shutdown();
		delete m_Point;
		m_Point = nullptr;
	}

	// Release the border bitmap object.
	if(m_Border)
	{
		m_Border->Shutdown();
		delete m_Border;
		m_Border = nullptr;
	}

	// Release the mini-map bitmap object.
	if(m_MiniMapBitmap)
	{
		m_MiniMapBitmap->Shutdown();
		delete m_MiniMapBitmap;
		m_MiniMapBitmap = nullptr;
	}

	return;
}


bool MiniMap::Render(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix, Shader* textureShader)
{
	bool result;


	// Put the border bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	result = m_Border->Render(deviceContext, (m_mapLocationX - 2), (m_mapLocationY - 2));
	if(!result)
	{
		return false;
	}

	// Render the border bitmap using the texture shader.
	textureShader->RenderTextureShader(deviceContext, m_Border->GetIndexCount(), worldMatrix, m_viewMatrix, orthoMatrix, m_Border->GetTexture());

	// Put the mini-map bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	result = m_MiniMapBitmap->Render(deviceContext, m_mapLocationX, m_mapLocationY);
	if(!result)
	{
		return false;
	}

	// Render the mini-map bitmap using the texture shader.
	textureShader->RenderTextureShader(deviceContext, m_MiniMapBitmap->GetIndexCount(), worldMatrix, m_viewMatrix, orthoMatrix, m_MiniMapBitmap->GetTexture());

	// Put the point bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	result = m_Point->Render(deviceContext, m_pointLocationX, m_pointLocationY);
	if(!result)
	{
		return false;
	}

	// Render the point bitmap using the texture shader.
	textureShader->RenderTextureShader(deviceContext, m_Point->GetIndexCount(), worldMatrix, m_viewMatrix, orthoMatrix, m_Point->GetTexture());

	return true;
}


void MiniMap::PositionUpdate(float positionX, float positionZ)
{
	float percentX, percentY;


	// Ensure the point does not leave the minimap borders even if the camera goes past the terrain borders.
	if(positionX < 0)
	{
		positionX = 0;
	}

	if(positionZ < 0)
	{
		positionZ = 0;
	}

	if(positionX > m_worldLength)
	{
		positionX = m_worldLength;
	}

	if(positionZ > m_worldWidth)
	{
		positionZ = m_worldWidth;
	}

	// Calculate the position of the camera on the minimap in terms of percentage.
	percentX = positionX / m_worldLength;
	percentY = 1.0f - (positionZ / m_worldWidth);

	// Determine the pixel location of the point on the mini-map.
	m_pointLocationX = m_mapLocationX + (int)(percentX * m_mapSizeX);
	m_pointLocationY = m_mapLocationY + (int)(percentY * m_mapSizeY);

	// Subtract one from the location to center the point on the mini-map according to the 3x3 point pixel image size.
	m_pointLocationX = m_pointLocationX - 1;
	m_pointLocationY = m_pointLocationY - 1;

	return;
}

} // end of namespace Gumshoe
