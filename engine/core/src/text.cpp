/*!
  @file
  text.cpp

  @brief
  Functionality for 2D text drawing to the screen.

  @detail
  Needs instantiations of Font and FontShader to work.
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "text.h"

#include "font.cpp"
//#include "font_shader.cpp"


namespace Gumshoe {

Text::Text(int numSentences)
{
	m_Font = nullptr;
	m_FontShader = nullptr;

	m_sentences.resize(numSentences, nullptr);
}


Text::~Text()
{
}


bool Text::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight, 
				D3DXMATRIX baseViewMatrix)
{
	bool result;


	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the base view matrix.
	m_baseViewMatrix = baseViewMatrix;

	// Create the font object.
	m_Font = new Font;
	if(!m_Font)
	{
		return false;
	}

	// Initialize the font object.
	// TODO(ebd): Texture filenames need to be parameterized
    LPCSTR fontTextureFilename = (LPCSTR)"../assets/font.dds";

	result = m_Font->Init(device, "../assets/font_data.txt", &fontTextureFilename);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the font object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	// Create the font shader object.
	m_FontShader = new FontShader;
	if(!m_FontShader)
	{
		return false;
	}

	// Initialize the font shader object.
	// TODO(ebd): Shader filenames need to be parameterized
    LPCSTR vsFilename = (LPCSTR)"../engine/core/inc/shaders/font.vs";
    LPCSTR psFilename = (LPCSTR)"../engine/core/inc/shaders/font.ps";

	result = m_FontShader->Init(device, hwnd, &vsFilename, &psFilename);
	if(!result)
	{
		MessageBox(hwnd, reinterpret_cast<LPCSTR>("Could not initialize the font shader object."), reinterpret_cast<LPCSTR>("Error"), MB_OK);
		return false;
	}

	uint32 length = 0;
	// loop through the sentences to initalize them
	for (int i = 0; i < m_sentences.size(); i++)
	{
        // Set the sentence length
        if (i == 0)
        {
        	length = 150;
        }
        else if (i == 1 || i == 10)
        {
        	length = 32;
        }
        else
        {
        	length = 16;
        }

        // Initialize the sentence.
		result = InitSentence(&m_sentences[i], length, device);
		if(!result)
		{
			return false;
		}

		// Now update the sentence vertex buffer with the new string information.
		result = UpdateSentence(m_sentences[i], "init_string", 20, 20*(i+1), 1.0f, 1.0f, 1.0f, deviceContext);
		if(!result)
		{
			return false;
		}
	}
	
	return true;
}


void Text::Shutdown()
{
	// Loop through the sentences to release them
	for (int i = 0; i < m_sentences.size(); i++)
	{
	    // Release the sentence.
	    ReleaseSentence(&m_sentences[i]);
	}

	// Release the font shader object.
	if(m_FontShader)
	{
		m_FontShader->Shutdown();
		delete m_FontShader;
		m_FontShader = nullptr;
	}

	// Release the font object.
	if(m_Font)
	{
		m_Font->Shutdown();
		delete m_Font;
		m_Font = nullptr;
	}

	return;
}


bool Text::Render(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
	bool result;


	// loop through the sentences to render them
	for (int i = 0; i < m_sentences.size(); i++)
	{
        // Draw the first sentence.
		result = RenderSentence(deviceContext, m_sentences[i], worldMatrix, orthoMatrix);
		if(!result)
		{
			return false;
		}
	}

	return true;
}


bool Text::InitSentence(textSentence_t** sentence, int maxLength, ID3D11Device* device)
{
	textVertex_t* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;


	// Create a new sentence object.
	*sentence = new textSentence_t;
	if(!*sentence)
	{
		return false;
	}

	// Initialize the sentence buffers to null.
	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;

	// Set the maximum length of the sentence.
	(*sentence)->maxLength = maxLength;

	// Set the number of vertices in the vertex array.
	(*sentence)->vertexCount = 6 * maxLength;

	// Set the number of indexes in the index array.
	(*sentence)->indexCount = (*sentence)->vertexCount;

	// Create the vertex array.
	vertices = new textVertex_t[(*sentence)->vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[(*sentence)->indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(textVertex_t) * (*sentence)->vertexCount));

	// Initialize the index array.
	for(i=0; i<(*sentence)->indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(textVertex_t) * (*sentence)->vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex array as it is no longer needed.
	delete [] vertices;
	vertices = nullptr;

	// Release the index array as it is no longer needed.
	delete [] indices;
	indices = nullptr;

	return true;
}


bool Text::UpdateSentence(textSentence_t* sentence, char* text, int positionX, int positionY, float red, float green, float blue,
						  ID3D11DeviceContext* deviceContext)
{
	int numLetters;
	textVertex_t* vertices;
	float drawX, drawY;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	textVertex_t* verticesPtr;


	// Store the color of the sentence.
	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	// Get the number of letters in the sentence.
	numLetters = (int)strlen(text);

	// Check for possible buffer overflow.
	if(numLetters > sentence->maxLength)
	{
		return false;
	}

	// Create the vertex array.
	vertices = new textVertex_t[sentence->vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(textVertex_t) * sentence->vertexCount));

	// Calculate the X and Y pixel position on the screen to start drawing to.
	drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
	drawY = (float)((m_screenHeight / 2) - positionY);

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

	// Lock the vertex buffer so it can be written to.
	result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (textVertex_t*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(textVertex_t) * sentence->vertexCount));

	// Unlock the vertex buffer.
	deviceContext->Unmap(sentence->vertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	delete [] vertices;
	vertices = nullptr;

	return true;
}


void Text::ReleaseSentence(textSentence_t** sentence)
{
	if(*sentence)
	{
		// Release the sentence vertex buffer.
		if((*sentence)->vertexBuffer)
		{
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = nullptr;
		}

		// Release the sentence index buffer.
		if((*sentence)->indexBuffer)
		{
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = nullptr;
		}

		// Release the sentence.
		delete *sentence;
		*sentence = nullptr;
	}

	return;
}


bool Text::RenderSentence(ID3D11DeviceContext* deviceContext, textSentence_t* sentence, D3DXMATRIX worldMatrix, 
						  D3DXMATRIX orthoMatrix)
{
	uint32 stride, offset;
	D3DXVECTOR4 pixelColor;
	bool result;


	// Set vertex buffer stride and offset.
    stride = sizeof(textVertex_t); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create a pixel color vector with the input sentence color.
	pixelColor = D3DXVECTOR4(sentence->red, sentence->green, sentence->blue, 1.0f);

	// Render the text using the font shader.
	result = m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(), 
								  pixelColor);
	if(!result)
	{
		false;
	}

	return true;
}


//--------------------------------------------
// Functions for setting various sentences
//--------------------------------------------
bool Text::SetMousePosition(int mouseX, int mouseY, ID3D11DeviceContext* deviceContext)
{
	char tempString[16];
	char mouseString[16];
	bool result;


	// Convert the mouseX integer to string format.
	_itoa_s(mouseX, tempString, 10);

	// Setup the mouseX string.
	strcpy_s(mouseString, "Mouse X: ");
	strcat_s(mouseString, tempString);

	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[0], mouseString, 20, 20, 1.0f, 1.0f, 1.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	// Convert the mouseY integer to string format.
	_itoa_s(mouseY, tempString, 10);

	// Setup the mouseY string.
	strcpy_s(mouseString, "Mouse Y: ");
	strcat_s(mouseString, tempString);

	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[1], mouseString, 20, 40, 1.0f, 1.0f, 1.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


bool Text::SetFps(int fps, ID3D11DeviceContext* deviceContext)
{
	char tempString[16];
	char fpsString[16];
	float red = 0.0f; 
	float green = 0.0f;
	float blue = 0.0f;
	bool result;


	// Truncate the fps to below 10,000.
	if(fps > 9999)
	{
		fps = 9999;
	}

	// Convert the fps integer to string format.
	_itoa_s(fps, tempString, 10);

	// Setup the fps string.
	strcpy_s(fpsString, "Fps: ");
	strcat_s(fpsString, tempString);

	// If fps is 60 or above set the fps color to green.
	if(fps >= 60)
	{
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// If fps is below 60 set the fps color to yellow.
	if(fps < 60)
	{
		red = 1.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// If fps is below 30 set the fps color to red.
	if(fps < 30)
	{
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
	}

	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[2], fpsString, 20, 20*3, red, green, blue, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


bool Text::SetFrameTime(float frameTime, ID3D11DeviceContext* deviceContext)
{
	char frameTimeString[16];
	float red = 0.0f; 
	float green = 0.0f;
	float blue = 0.0f;
	bool result;


	// Setup the frame time string.
	_snprintf_s(frameTimeString, sizeof(frameTimeString),
                "Ms/f: %.02f", frameTime);
	
	// If frame time is below 16.7 ms (60 fps), set the color to green.
	if(frameTime <= 17.0f)
	{
		red = 0.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// If frame time is below 33.4 ms (30 fps), set the color to yellow.
	else if(frameTime <= 33.5f)
	{
		red = 1.0f;
		green = 1.0f;
		blue = 0.0f;
	}

	// If frame time is larger than 33.4 ms (less than 30 fps), set the color to red
	else
	{
		red = 1.0f;
		green = 0.0f;
		blue = 0.0f;
	}

	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[1], frameTimeString, 20, 40, red, green, blue, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


bool Text::SetRenderCount(int renderCount, ID3D11DeviceContext* deviceContext)
{
	char renderCountString[32];
	bool result;


	// Setup the frame time string.
	_snprintf_s(renderCountString, sizeof(renderCountString),
                "Render Count: %d", renderCount);
	
	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[10], renderCountString, 20, 20*11, 1.0f, 1.0f, 1.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


bool Text::SetCpu(int cpu, ID3D11DeviceContext* deviceContext)
{
	char tempString[16];
	char cpuString[16];
	bool result;


	// Convert the cpu integer to string format.
	_itoa_s(cpu, tempString, 10);

	// Setup the cpu string.
	strcpy_s(cpuString, "Cpu: ");
	strcat_s(cpuString, tempString);
	strcat_s(cpuString, "%");

	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[3], cpuString, 20, 20*4, 0.0f, 1.0f, 0.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


bool Text::SetVideoCardInfo(char* videoCardName, int videoCardMemory, ID3D11DeviceContext* deviceContext)
{
	char dataString[150];
	bool result;
	char tempString[16];
	char memoryString[32];


	// Setup the video card name string.
	strcpy_s(dataString, "Video Card: ");
	strcat_s(dataString, videoCardName);

	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[0], dataString, 20, 20, 1.0f, 1.0f, 1.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	// Truncate the memory value to prevent buffer over flow.
	if(videoCardMemory > 9999999)
	{
		videoCardMemory = 9999999;
	}

	// Convert the video memory integer value to a string format.
	_itoa_s(videoCardMemory, tempString, 10);

	// Setup the video memory string.
	strcpy_s(memoryString, "Video Memory: ");
	strcat_s(memoryString, tempString);
	strcat_s(memoryString, " MB");

	// Update the sentence vertex buffer with the new string information.
	result = UpdateSentence(m_sentences[1], memoryString, 20, 40, 1.0f, 1.0f, 1.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


bool Text::SetCameraPosition(float posX, float posY, float posZ, ID3D11DeviceContext* deviceContext)
{
	int positionX, positionY, positionZ;
	char tempString[16];
	char dataString[16];
	bool result;


	// Convert the position from floating point to integer.
	positionX = (int)posX;
	positionY = (int)posY;
	positionZ = (int)posZ;

	// Truncate the position if it exceeds either 9999 or -9999.
	if(positionX > 9999) { positionX = 9999; }
	if(positionY > 9999) { positionY = 9999; }
	if(positionZ > 9999) { positionZ = 9999; }

	if(positionX < -9999) { positionX = -9999; }
	if(positionY < -9999) { positionY = -9999; }
	if(positionZ < -9999) { positionZ = -9999; }

	// Setup the X position string.
	_itoa_s(positionX, tempString, 10);
	strcpy_s(dataString, "X: ");
	strcat_s(dataString, tempString);

	result = UpdateSentence(m_sentences[4], dataString, 20, 20*5, 0.0f, 1.0f, 0.0f, deviceContext);
	if(!result)
	{
		return false;
	}
	
	// Setup the Y position string.
	_itoa_s(positionY, tempString, 10);
	strcpy_s(dataString, "Y: ");
	strcat_s(dataString, tempString);

	result = UpdateSentence(m_sentences[5], dataString, 20, 20*6, 0.0f, 1.0f, 0.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	// Setup the Z position string.
	_itoa_s(positionZ, tempString, 10);
	strcpy_s(dataString, "Z: ");
	strcat_s(dataString, tempString);

	result = UpdateSentence(m_sentences[6], dataString, 20, 20*7, 0.0f, 1.0f, 0.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


bool Text::SetCameraRotation(float rotX, float rotY, float rotZ, ID3D11DeviceContext* deviceContext)
{
	int rotationX, rotationY, rotationZ;
	char tempString[16];
	char dataString[16];
	bool result;


	// Convert the rotation from floating point to integer.
	rotationX = (int)rotX;
	rotationY = (int)rotY;
	rotationZ = (int)rotZ;

	// Setup the X rotation string.
	_itoa_s(rotationX, tempString, 10);
	strcpy_s(dataString, "rX: ");
	strcat_s(dataString, tempString);

	result = UpdateSentence(m_sentences[7], dataString, 20, 20*8, 0.0f, 1.0f, 0.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	// Setup the Y rotation string.
	_itoa_s(rotationY, tempString, 10);
	strcpy_s(dataString, "rY: ");
	strcat_s(dataString, tempString);

	result = UpdateSentence(m_sentences[8], dataString, 20, 20*9, 0.0f, 1.0f, 0.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	// Setup the Z rotation string.
	_itoa_s(rotationZ, tempString, 10);
	strcpy_s(dataString, "rZ: ");
	strcat_s(dataString, tempString);

	result = UpdateSentence(m_sentences[9], dataString, 20, 20*10, 0.0f, 1.0f, 0.0f, deviceContext);
	if(!result)
	{
		return false;
	}

	return true;
}


} // end of namespace Gumshoe