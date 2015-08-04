/*!
  @file
  font.cpp

  @brief
  Functionality for creating a 2D font object.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "font.h"

#include "texture.cpp"


namespace Gumshoe {

Font::Font()
{
	m_fontChars = nullptr;
	m_Texture = nullptr;
}


Font::~Font()
{
}


bool Font::Init(ID3D11Device* device, char* fontFilename, LPCSTR* textureFilename)
{
	bool result;


	// Load in the text file containing the font data.
	result = LoadFontData(fontFilename);
	if(!result)
	{
		return false;
	}

	// Load the texture that has the font characters on it.
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


void Font::Shutdown()
{
	// Release the font texture.
	ReleaseTexture();

	// Release the font data.
	ReleaseFontData();

	return;
}


bool Font::LoadFontData(char* filename)
{
	ifstream fin;
	int i;
	char temp;


	// Create the font spacing buffer.
	m_fontChars = new fontCharData_t[95];
	if(!m_fontChars)
	{
		return false;
	}

	// Read in the font size and spacing between chars.
	fin.open(filename);
	if(fin.fail())
	{
		return false;
	}

	// Read in the 95 used ascii characters for text.
	for(i=0; i<95; i++)
	{
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}

		fin >> m_fontChars[i].left;
		fin >> m_fontChars[i].right;
		fin >> m_fontChars[i].size;
	}

	// Close the file.
	fin.close();

	return true;
}


void Font::ReleaseFontData()
{
	// Release the font data array.
	if(m_fontChars)
	{
		delete [] m_fontChars;
		m_fontChars = nullptr;
	}

	return;
}


bool Font::LoadTexture(ID3D11Device* device, LPCSTR* filename)
{
	bool result;


	// Create the texture object.
	m_Texture = new Texture;
	if(!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_Texture->Init(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}


void Font::ReleaseTexture()
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}

	return;
}


ID3D11ShaderResourceView* Font::GetTexture()
{
	return m_Texture->GetTexture();
}


void Font::BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY)
{
	fontVertex_t* vertexPtr;
	int numChars, index, i, currChar;


	// Coerce the input vertices into a fontVertex_t structure.
	vertexPtr = (fontVertex_t*)vertices;

	// Get the number of letters in the sentence.
	numChars = (int)strlen(sentence);

	// Initialize the index to the vertex array.
	index = 0;

	// Draw each character onto a quad.
	for(i=0; i<numChars; i++)
	{
		currChar = ((int)sentence[i]) - 32;

		// If the current character is a space then just move over three pixels.
		if(currChar == 0)
		{
			drawX = drawX + 3.0f;
		}
		else
		{
			// First triangle in quad.
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = D3DXVECTOR2(m_fontChars[currChar].left, 0.0f);
			index++;

			vertexPtr[index].position = D3DXVECTOR3((drawX + m_fontChars[currChar].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = D3DXVECTOR2(m_fontChars[currChar].right, 1.0f);
			index++;

			vertexPtr[index].position = D3DXVECTOR3(drawX, (drawY - 16), 0.0f);  // Bottom left.
			vertexPtr[index].texture = D3DXVECTOR2(m_fontChars[currChar].left, 1.0f);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = D3DXVECTOR2(m_fontChars[currChar].left, 0.0f);
			index++;

			vertexPtr[index].position = D3DXVECTOR3(drawX + m_fontChars[currChar].size, drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = D3DXVECTOR2(m_fontChars[currChar].right, 0.0f);
			index++;

			vertexPtr[index].position = D3DXVECTOR3((drawX + m_fontChars[currChar].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = D3DXVECTOR2(m_fontChars[currChar].right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the character and one pixel.
			drawX = drawX + m_fontChars[currChar].size + 1.0f;
		}
	}

	return;
}


} // end of namespace Gumshoe