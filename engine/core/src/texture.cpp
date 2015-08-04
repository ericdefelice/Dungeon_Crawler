/*!
  @file
  texture.cpp

   @brief
  Functionality for DirectX textures.

  @detail
  Encapsulates the loading, unloading, and accessing of a single texture resource. For each texture needed an object of this class must be instantiated.
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "texture.h"


namespace Gumshoe {

Texture::Texture()
{
	m_texture = nullptr;
}


Texture::~Texture()
{
}


bool Texture::Init(ID3D11Device* device, LPCSTR* filename)
{
	HRESULT result;


	// Load the texture in.
	result = D3DX11CreateShaderResourceViewFromFile(device, *filename, NULL, NULL, &m_texture, NULL);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void Texture::Shutdown()
{
	// Release the texture resource.
	if(m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}

	return;
}


ID3D11ShaderResourceView* Texture::GetTexture()
{
	return m_texture;
}

} // end of namespace Gumshoe
