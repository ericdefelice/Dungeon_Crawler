/*!
  @file
  skyplane.cpp

  @brief
  Functionality for creating the plane where the clouds will be rendered on.

  @detail

*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "skyplane.h"


namespace Gumshoe {

SkyPlane::SkyPlane()
{
	m_skyPlane = nullptr;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_CloudTexture = nullptr;
	m_PerturbTexture = nullptr;
}


SkyPlane::~SkyPlane()
{
}


bool SkyPlane::Init(ID3D11Device* device, LPCSTR* cloudTextureFilename, LPCSTR* perturbTextureFilename)
{
	int skyPlaneResolution, textureRepeat;
	float skyPlaneWidth, skyPlaneTop, skyPlaneBottom;
	bool result;


	// Set the sky plane parameters.
	skyPlaneResolution = 50;
	skyPlaneWidth = 10.0f;
	skyPlaneTop = 0.5f;
	skyPlaneBottom = 0.0f;
	textureRepeat = 2;

	// Set the sky plane shader related parameters.
	m_scale = 0.3f;
	m_brightness = 0.5f;

	// Initialize the translation to zero.
	m_translation = 0.0f;

	// Create the sky plane.
	result = InitializeSkyPlane(skyPlaneResolution, skyPlaneWidth, skyPlaneTop, skyPlaneBottom, textureRepeat);
	if(!result)
	{
		return false;
	}

	// Create the vertex and index buffer for the sky plane.
	result = InitializeBuffers(device, skyPlaneResolution);
	if(!result)
	{
		return false;
	}

	// Load the sky plane textures.
	result = LoadTextures(device, cloudTextureFilename, perturbTextureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


void SkyPlane::Shutdown()
{
	// Release the sky plane textures.
	ReleaseTextures();

	// Release the vertex and index buffer that were used for rendering the sky plane.
	ShutdownBuffers();

	// Release the sky plane array.
	ShutdownSkyPlane();

	return;
}


void SkyPlane::Render(ID3D11DeviceContext* deviceContext)
{
	// Render the sky plane.
	RenderBuffers(deviceContext);

	return;
}


void SkyPlane::Frame(float frameTime)
{
	// Increment the texture translation value each frame.
	m_translation += 0.0001f;
	if(m_translation > 1.0f)
	{
		m_translation -= 1.0f;
	}

	return;
}


int SkyPlane::GetIndexCount()
{
	return m_indexCount;
}


ID3D11ShaderResourceView* SkyPlane::GetCloudTexture()
{
	return m_CloudTexture->GetTexture();
}


ID3D11ShaderResourceView* SkyPlane::GetPerturbTexture()
{
	return m_PerturbTexture->GetTexture();
}


float SkyPlane::GetScale()
{
	return m_scale;
}


float SkyPlane::GetBrightness()
{

	return m_brightness;
}


float SkyPlane::GetTranslation()
{

	return m_translation;
}


bool SkyPlane::InitializeSkyPlane(int skyPlaneResolution, float skyPlaneWidth, float skyPlaneTop, float skyPlaneBottom, int textureRepeat)
{
	float quadSize, radius, constant, textureDelta;
	int i, j, index;
	float positionX, positionY, positionZ, tu, tv;


	// Create the array to hold the sky plane coordinates.
	m_skyPlane = new skyPlane_t[(skyPlaneResolution + 1) * (skyPlaneResolution + 1)];
	if(!m_skyPlane)
	{
		return false;
	}

	// Determine the size of each quad on the sky plane.
	quadSize = skyPlaneWidth / (float)skyPlaneResolution;

	// Calculate the radius of the sky plane based on the width.
	radius = skyPlaneWidth / 2.0f;

	// Calculate the height constant to increment by.
	constant = (skyPlaneTop - skyPlaneBottom) / (radius * radius);

	// Calculate the texture coordinate increment value.
	textureDelta = (float)textureRepeat / (float)skyPlaneResolution;

	// Loop through the sky plane and build the coordinates based on the increment values given.
	for(j=0; j<=skyPlaneResolution; j++)
	{
		for(i=0; i<=skyPlaneResolution; i++)
		{
			// Calculate the vertex coordinates.
			positionX = (-0.5f * skyPlaneWidth) + ((float)i * quadSize);
			positionZ = (-0.5f * skyPlaneWidth) + ((float)j * quadSize);
			positionY = skyPlaneTop - (constant * ((positionX * positionX) + (positionZ * positionZ)));

			// Calculate the texture coordinates.
			tu = (float)i * textureDelta;
			tv = (float)j * textureDelta;

			// Calculate the index into the sky plane array to add this coordinate.
			index = j * (skyPlaneResolution + 1) + i;

			// Add the coordinates to the sky plane array.
			m_skyPlane[index].x = positionX;
			m_skyPlane[index].y = positionY;
			m_skyPlane[index].z = positionZ;
			m_skyPlane[index].tu = tu;
			m_skyPlane[index].tv = tv;
		}
	}

	return true;
}


void SkyPlane::ShutdownSkyPlane()
{
	// Release the sky plane array.
	if(m_skyPlane)
	{
		delete [] m_skyPlane;
		m_skyPlane = nullptr;
	}

	return;
}


bool SkyPlane::InitializeBuffers(ID3D11Device* device, int skyPlaneResolution)
{
	vertex_t* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i, j, index, index1, index2, index3, index4;


	// Calculate the number of vertices in the sky plane mesh.
	m_vertexCount = (skyPlaneResolution + 1) * (skyPlaneResolution + 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;
		
	// Create the vertex array.
	vertices = new vertex_t[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize the index into the vertex array.
	index = 0;

	// Load the vertex and index array with the sky plane array data.
	for(j=0; j<skyPlaneResolution; j++)
	{
		for(i=0; i<skyPlaneResolution; i++)
		{
			index1 = j * (skyPlaneResolution + 1) + i;
			index2 = j * (skyPlaneResolution + 1) + (i+1);
			index3 = (j+1) * (skyPlaneResolution + 1) + i;
			index4 = (j+1) * (skyPlaneResolution + 1) + (i+1);

			// Triangle 1 - Upper Left
			vertices[index].position = D3DXVECTOR3(m_skyPlane[index1].x, m_skyPlane[index1].y, m_skyPlane[index1].z);
			vertices[index].texture = D3DXVECTOR2(m_skyPlane[index1].tu, m_skyPlane[index1].tv);
			indices[index] = index;
			index++;

			// Triangle 1 - Upper Right
			vertices[index].position = D3DXVECTOR3(m_skyPlane[index2].x, m_skyPlane[index2].y, m_skyPlane[index2].z);
			vertices[index].texture = D3DXVECTOR2(m_skyPlane[index2].tu, m_skyPlane[index2].tv);
			indices[index] = index;
			index++;

			// Triangle 1 - Bottom Left
			vertices[index].position = D3DXVECTOR3(m_skyPlane[index3].x, m_skyPlane[index3].y, m_skyPlane[index3].z);
			vertices[index].texture = D3DXVECTOR2(m_skyPlane[index3].tu, m_skyPlane[index3].tv);
			indices[index] = index;
			index++;

			// Triangle 2 - Bottom Left
			vertices[index].position = D3DXVECTOR3(m_skyPlane[index3].x, m_skyPlane[index3].y, m_skyPlane[index3].z);
			vertices[index].texture = D3DXVECTOR2(m_skyPlane[index3].tu, m_skyPlane[index3].tv);
			indices[index] = index;
			index++;

			// Triangle 2 - Upper Right
			vertices[index].position = D3DXVECTOR3(m_skyPlane[index2].x, m_skyPlane[index2].y, m_skyPlane[index2].z);
			vertices[index].texture = D3DXVECTOR2(m_skyPlane[index2].tu, m_skyPlane[index2].tv);
			indices[index] = index;
			index++;

			// Triangle 2 - Bottom Right
			vertices[index].position = D3DXVECTOR3(m_skyPlane[index4].x, m_skyPlane[index4].y, m_skyPlane[index4].z);
			vertices[index].texture = D3DXVECTOR2(m_skyPlane[index4].tu, m_skyPlane[index4].tv);
			indices[index] = index;
			index++;
		}
	}

	// Set up the description of the vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(vertex_t) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = nullptr;

	delete [] indices;
	indices = nullptr;

	return true;
}


void SkyPlane::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	return;
}


void SkyPlane::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
    stride = sizeof(vertex_t); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


bool SkyPlane::LoadTextures(ID3D11Device* device, LPCSTR* textureFilename1, LPCSTR* textureFilename2)
{
	bool result;


	// Create the first cloud texture object.
	m_CloudTexture = new Texture;
	if(!m_CloudTexture)
	{
		return false;
	}

	// Initialize the first cloud texture object.
	result = m_CloudTexture->Init(device, textureFilename1);
	if(!result)
	{
		return false;
	}

	// Create the second cloud texture object.
	m_PerturbTexture = new Texture;
	if(!m_PerturbTexture)
	{
		return false;
	}

	// Initialize the second cloud texture object.
	result = m_PerturbTexture->Init(device, textureFilename2);
	if(!result)
	{
		return false;
	}

	return true;
}


void SkyPlane::ReleaseTextures()
{
	// Release the texture objects.
	if(m_PerturbTexture)
	{
		m_PerturbTexture->Shutdown();
		delete m_PerturbTexture;
		m_PerturbTexture = nullptr;
	}

	if(m_CloudTexture)
	{
		m_CloudTexture->Shutdown();
		delete m_CloudTexture;
		m_CloudTexture = nullptr;
	}

	return;
}

} // end of namespace Gumshoe
