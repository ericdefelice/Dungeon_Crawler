/*!
  @file
  font.h

  @brief
  Functionality for creating a 2D font object.

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
#include "texture.h"

using namespace std;

namespace Gumshoe {

//--------------------------------------------
// Font class definition
//--------------------------------------------
class Font
{
private:
	struct fontCharData_t
	{
		float left, right;
		int size;
	};

	struct fontVertex_t
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	Font();
	~Font();

	bool Init(ID3D11Device*, char*, LPCSTR*);
	void Shutdown();
	
	ID3D11ShaderResourceView* GetTexture();

    void BuildVertexArray(void*, char*, float, float);

private:
	bool LoadFontData(char*);
	void ReleaseFontData();
	bool LoadTexture(ID3D11Device*, LPCSTR*);
	void ReleaseTexture();

private:
	fontCharData_t* m_fontChars;
	Texture* m_Texture;
};

} // end of namespace Gumshoe