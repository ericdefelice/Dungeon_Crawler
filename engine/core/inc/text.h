/*!
  @file
  text.h

  @brief
  Functionality for 2D text drawing to the screen.

  @detail
  Needs instantiations of Font and FontShader to work.
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "font.h"
#include "font_shader.h"

#include <vector>


namespace Gumshoe {

//--------------------------------------------
// Text class definition
//--------------------------------------------
class Text
{
private:
	struct textSentence_t
	{
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount, maxLength;
		float red, green, blue;
	};

	struct textVertex_t
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	Text(int);
	~Text();

	bool Init(ID3D11Device*, ID3D11DeviceContext*, HWND, int, int, D3DXMATRIX);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX);

	bool SetMousePosition(int, int, ID3D11DeviceContext*);
	bool SetFrameTime(float, ID3D11DeviceContext*);
	bool SetRenderCount(int, ID3D11DeviceContext*);

	bool SetVideoCardInfo(char*, int, ID3D11DeviceContext*);
	bool SetFps(int, ID3D11DeviceContext*);
	bool SetCpu(int, ID3D11DeviceContext*);
	bool SetCameraPosition(float, float, float, ID3D11DeviceContext*);
	bool SetCameraRotation(float, float, float, ID3D11DeviceContext*);
	
private:
	bool InitSentence(textSentence_t**, int, ID3D11Device*);
	bool UpdateSentence(textSentence_t*, char*, int, int, float, float, float, ID3D11DeviceContext*);
	void ReleaseSentence(textSentence_t**);
	bool RenderSentence(ID3D11DeviceContext*, textSentence_t*, D3DXMATRIX, D3DXMATRIX);

private:
	int m_screenWidth, m_screenHeight;
	D3DXMATRIX m_baseViewMatrix;
	vector<textSentence_t*> m_sentences;

	Font* m_Font;
	FontShader* m_FontShader;
};

} // end of namespace Gumshoe