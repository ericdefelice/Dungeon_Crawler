/*!
  @file
  texture.h

  @brief
  Functionality for DirectX textures.

  @detail
  Encapsulates the loading, unloading, and accessing of a single texture resource. For each texture needed an object of this class must be instantiated.
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include <d3d11.h>
#include <d3dx11tex.h>


namespace Gumshoe {

//--------------------------------------------
// Texture class definition
//--------------------------------------------
class Texture
{
public:
	Texture();
	~Texture();

	bool Init(ID3D11Device*, LPCSTR*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	ID3D11ShaderResourceView* m_texture;
};

} // end of namespace Gumshoe
