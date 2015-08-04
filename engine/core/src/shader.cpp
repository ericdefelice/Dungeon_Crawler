/*!
  @file
  shader.cpp

  @brief
  Functionality for the DirectX shaders.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "shader.h"

using namespace std;

namespace Gumshoe {

Shader::Shader()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_sampleState = nullptr;
	m_matrixBuffer = nullptr;
	m_cameraBuffer = nullptr;
	m_lightBuffer = nullptr;
	m_textureInfoBuffer = nullptr;
}


Shader::~Shader()
{
}


bool Shader::Init(ID3D11Device* device, HWND hwnd, LPCSTR* vsFilename, LPCSTR* psFilename, int shaderType)
{
	bool result = 0;

    // Initialize the vertex and pixel shaders.
    // If it is a color shader
	if (shaderType == 0)
	{
        result = InitColorShader(device, hwnd, vsFilename, psFilename);
	} 
	// If it is a ambient light shader
	else if (shaderType == 1)
	{
		const LPCSTR vsFunctionName = (LPCSTR)"AmbientLightVertexShader";
		const LPCSTR psFunctionName = (LPCSTR)"AmbientLightPixelShader";
		result = InitShader(device, hwnd, vsFilename, psFilename, &vsFunctionName, &psFunctionName, shaderType);	
	}
	// If it is an specular light shader
	else if (shaderType == 2)
	{
		const LPCSTR vsFunctionName = (LPCSTR)"SpecularLightVertexShader";
		const LPCSTR psFunctionName = (LPCSTR)"SpecularLightPixelShader";
		result = InitShader(device, hwnd, vsFilename, psFilename, &vsFunctionName, &psFunctionName, shaderType);
	}
	// If it is a color & ambient light shader
	else if (shaderType == 3)
	{
		const LPCSTR vsFunctionName = (LPCSTR)"ColorAmbientLightVertexShader";
		const LPCSTR psFunctionName = (LPCSTR)"ColorAmbientLightPixelShader";
		result = InitShader(device, hwnd, vsFilename, psFilename, &vsFunctionName, &psFunctionName, shaderType);
	}
	// If it is a texture shader
	else if (shaderType == 4)
	{
		const LPCSTR vsFunctionName = (LPCSTR)"TextureVertexShader";
		const LPCSTR psFunctionName = (LPCSTR)"TexturePixelShader";
		result = InitShader(device, hwnd, vsFilename, psFilename, &vsFunctionName, &psFunctionName, shaderType);
	}
	
	if(!result)
	{
		return false;
	}

	return true;
}


void Shader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}


// ----------------------------------------------------
// Shader Render function for color shader
// ----------------------------------------------------
bool Shader::RenderColorShader(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
					           D3DXMATRIX projectionMatrix)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetColorShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


// ----------------------------------------------------
// Shader Render function for texture shader
// ----------------------------------------------------
bool Shader::RenderTextureShader(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
					             D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetTextureShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


// ----------------------------------------------------
// Shader Render function for ambient light shader
// ----------------------------------------------------
bool Shader::RenderAmbientShader(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
					             D3DXMATRIX projectionMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColor, 
					             D3DXVECTOR4 diffuseColor, D3DXVECTOR3 cameraPosition, ID3D11ShaderResourceView* groundTexture,
					             ID3D11ShaderResourceView* wallTexture)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetAmbientShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, lightDirection,
		                                ambientColor, diffuseColor, cameraPosition, groundTexture, wallTexture);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


// ----------------------------------------------------
// Shader Render function for specular light shader
// ----------------------------------------------------
bool Shader::RenderSpecularShader(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
					              D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColor,
					              D3DXVECTOR4 diffuseColor, D3DXVECTOR3 cameraPosition, D3DXVECTOR4 specularColor, float specularPower)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetSpecularShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambientColor,
		                                 diffuseColor, cameraPosition, specularColor, specularPower);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


// ----------------------------------------------------
// Public function for setting shader parameters
// ----------------------------------------------------
bool Shader::PublicSetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
					                   D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection,
					                   D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 cameraPosition, 
					                   D3DXVECTOR4 specularColor, float specularPower, int shaderType)
{
	bool result = false;


	// COLOR SHADER
	if (shaderType == 0)
	{
        result = false;
	}
	// AMBIENT LIGHT SHADER or COLOR AMBIENT LIGHT SHADER
	else if (shaderType == 1 || shaderType == 3)
	{
		// Set the shader parameters that it will use for rendering.
		//result = SetAmbientShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection,
		//	                                ambientColor, diffuseColor, cameraPosition);
	}
	// SPECULAR LIGHT SHADER
	else if (shaderType == 2)
	{
		// Set the shader parameters that it will use for rendering.
		result = SetSpecularShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, lightDirection, ambientColor,
		                                     diffuseColor, cameraPosition, specularColor, specularPower);
	}
	
	if(!result)
	{
		return false;
	}

	return true;
}


// ----------------------------------------------------
// Public function for rendering a shader
// ----------------------------------------------------
void Shader::PublicRenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}


bool Shader::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
								 D3DXMATRIX projectionMatrix, D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	shaderMatrixBuffer_t* matrixBufPtr;
	ambientLightBuffer_t* lightBufPtr;


	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	matrixBufPtr = (shaderMatrixBuffer_t*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufPtr->world = worldMatrix;
	matrixBufPtr->view = viewMatrix;
	matrixBufPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	lightBufPtr = (ambientLightBuffer_t*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	lightBufPtr->ambientColor = ambientColor;
	lightBufPtr->diffuseColor = diffuseColor;
	lightBufPtr->lightDirection = lightDirection;
	lightBufPtr->padding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	return true;
}


bool Shader::SetShaderTextures(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture1, 
							   ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* alphaMap, bool useAlpha)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	textureInfoBuffer_t* dataPtr;
	unsigned int bufferNumber;


	// Set shader texture resources in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture1);

	// If this is a blended polygon then also set the second texture and the alpha map for blending.
	if(useAlpha)
	{
		deviceContext->PSSetShaderResources(1, 1, &texture2);
		deviceContext->PSSetShaderResources(2, 1, &alphaMap);
	}

	// Lock the texture info constant buffer so it can be written to.
	result = deviceContext->Map(m_textureInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (textureInfoBuffer_t*)mappedResource.pData;

	// Copy the texture info variables into the constant buffer.
	dataPtr->useAlplha = useAlpha;
	dataPtr->padding2 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	// Unlock the constant buffer.
	deviceContext->Unmap(m_textureInfoBuffer, 0);

	// Set the position of the texture info constant buffer in the pixel shader.
	bufferNumber = 1;

	// Finally set the texture info constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_textureInfoBuffer);

	return true;
}


bool Shader::InitShader(ID3D11Device* device, HWND hwnd, LPCSTR* vsFilename, LPCSTR* psFilename, const LPCSTR* vsFunction, const LPCSTR* psFunction, int shaderType)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	uint32 numElements;
	D3D11_SAMPLER_DESC samplerDesc;
    D3D11_INPUT_ELEMENT_DESC stdPolygonLayout[3];
    D3D11_INPUT_ELEMENT_DESC texPolygonLayout[2];

	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	//D3D11_BUFFER_DESC textureInfoBufferDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile(*vsFilename, NULL, NULL, *vsFunction, "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, *vsFilename, reinterpret_cast<LPCSTR>("Missing Shader File"), MB_OK);
		}

		return false;
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile(*psFilename, NULL, NULL, *psFunction, "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, *psFilename, reinterpret_cast<LPCSTR>("Missing Shader File"), MB_OK);
		}

		return false;
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}

	
	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the Model and in the shader.
	
	// If this is a texture shader, use the texture layout array
    if (shaderType == 4)
    {
        texPolygonLayout[0].SemanticName = "POSITION";
		texPolygonLayout[0].SemanticIndex = 0;
		texPolygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		texPolygonLayout[0].InputSlot = 0;
		texPolygonLayout[0].AlignedByteOffset = 0;
		texPolygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		texPolygonLayout[0].InstanceDataStepRate = 0;

	    texPolygonLayout[1].SemanticName = "TEXCOORD";
		texPolygonLayout[1].SemanticIndex = 0;
		texPolygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		texPolygonLayout[1].InputSlot = 0;
		texPolygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		texPolygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		texPolygonLayout[1].InstanceDataStepRate = 0;

		numElements = sizeof(texPolygonLayout) / sizeof(texPolygonLayout[0]);

		// Create the vertex input layout.
		result = device->CreateInputLayout(texPolygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
										   vertexShaderBuffer->GetBufferSize(), &m_layout);
		if(FAILED(result))
		{
			return false;
		}
    }
    else
    {
    	stdPolygonLayout[0].SemanticName = "POSITION";
		stdPolygonLayout[0].SemanticIndex = 0;
		stdPolygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		stdPolygonLayout[0].InputSlot = 0;
		stdPolygonLayout[0].AlignedByteOffset = 0;
		stdPolygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		stdPolygonLayout[0].InstanceDataStepRate = 0;

	    stdPolygonLayout[1].SemanticName = "TEXCOORD";
		stdPolygonLayout[1].SemanticIndex = 0;
		stdPolygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		stdPolygonLayout[1].InputSlot = 0;
		stdPolygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		stdPolygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		stdPolygonLayout[1].InstanceDataStepRate = 0;

    	stdPolygonLayout[2].SemanticName = "NORMAL";
		stdPolygonLayout[2].SemanticIndex = 0;
		stdPolygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		stdPolygonLayout[2].InputSlot = 0;
		stdPolygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		stdPolygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		stdPolygonLayout[2].InstanceDataStepRate = 0;

		numElements = sizeof(stdPolygonLayout) / sizeof(stdPolygonLayout[0]);

		// Create the vertex input layout.
		result = device->CreateInputLayout(stdPolygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
										   vertexShaderBuffer->GetBufferSize(), &m_layout);
		if(FAILED(result))
		{
			return false;
		}
    }

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

    // Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
	{
		return false;
	}

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(shaderMatrixBuffer_t);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

	if (shaderType != 4)
	{
        // Setup the description of the camera dynamic constant buffer that is in the vertex shader.
		cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		cameraBufferDesc.ByteWidth = sizeof(shaderCameraBuffer_t);
		cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cameraBufferDesc.MiscFlags = 0;
		cameraBufferDesc.StructureByteStride = 0;

		// Create the camera constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&cameraBufferDesc, NULL, &m_cameraBuffer);
		if(FAILED(result))
		{
			return false;
		}

	    // Setup the description of the light dynamic constant buffer that is in the pixel shader.
		// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
		lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		// AMBIENT LIGHT SHADER or COLOR AMBIENT LIGHT SHADER
		if (shaderType == 1 || shaderType == 3)
		{
	        lightBufferDesc.ByteWidth = sizeof(ambientLightBuffer_t);
		}
		// SPECULAR LIGHT SHADER
		else if (shaderType == 2)
		{
			lightBufferDesc.ByteWidth = sizeof(specularLightBuffer_t);
		}
		lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		lightBufferDesc.MiscFlags = 0;
		lightBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
		if(FAILED(result))
		{
			return false;
		}
/*
		// Setup the description of the texture info constant buffer that is in the pixel shader.
		textureInfoBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		textureInfoBufferDesc.ByteWidth = sizeof(textureInfoBuffer_t);
		textureInfoBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		textureInfoBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		textureInfoBufferDesc.MiscFlags = 0;
		textureInfoBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the pixel shader constant buffer from within this class.
		result = device->CreateBuffer(&textureInfoBufferDesc, NULL, &m_textureInfoBuffer);
		if(FAILED(result))
		{
			return false;
		}
*/
	}

	return true;
}


bool Shader::InitColorShader(ID3D11Device* device, HWND hwnd, LPCSTR* vsFilename, LPCSTR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Compile the vertex shader code.
	result = D3DX11CompileFromFile(*vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, *vsFilename, reinterpret_cast<LPCSTR>("Missing Shader File"), MB_OK);
		}

		return false;
	}

    // Compile the pixel shader code.
	result = D3DX11CompileFromFile(*psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, *psFilename, reinterpret_cast<LPCSTR>("Missing Shader File"), MB_OK);
		}

		return false;
	}

    // Create the vertex shader from the buffer.
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
									   vertexShaderBuffer->GetBufferSize(), &m_layout);
	if(FAILED(result))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(shaderMatrixBuffer_t);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void Shader::ShutdownShader()
{
	// Release the texture info constant buffer.
	if(m_textureInfoBuffer)
	{
		m_textureInfoBuffer->Release();
		m_textureInfoBuffer = nullptr;
	}

	// Release the sampler state.
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = nullptr;
	}

	// Release the matrix constant buffer.
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}

	// Release the camera constant buffer.
	if(m_cameraBuffer)
	{
		m_cameraBuffer->Release();
		m_cameraBuffer = nullptr;
	}

	// Release the light constant buffer.
	if(m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = nullptr;
	}

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = nullptr;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}

	return;
}


void Shader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, LPCSTR* shaderFilename)
{
	char* compileErrors;
	uint32 bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = (uint32)errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = nullptr;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, reinterpret_cast<LPCSTR>("Error compiling shader. Check shader-error.txt for message."), *shaderFilename, MB_OK);

	return;
}


// -----------------------------------------------------------
// Shader SetParameters function for color shader
// -----------------------------------------------------------
bool Shader::SetColorShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
					          		  D3DXMATRIX projectionMatrix)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	shaderMatrixBuffer_t* matrixBufferPtr;
	uint32 bufferNumber;


	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	matrixBufferPtr = (shaderMatrixBuffer_t*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = worldMatrix;
	matrixBufferPtr->view = viewMatrix;
	matrixBufferPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}


// -----------------------------------------------------------
// Shader SetParameters function for texture shader
// -----------------------------------------------------------
bool Shader::SetTextureShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
						       	        D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	shaderMatrixBuffer_t* matrixBufferPtr;
	uint32 bufferNumber;


	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	matrixBufferPtr = (shaderMatrixBuffer_t*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = worldMatrix;
	matrixBufferPtr->view = viewMatrix;
	matrixBufferPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}


// -----------------------------------------------------------
// Shader SetParameters function for ambient light shader
// -----------------------------------------------------------
bool Shader::SetAmbientShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
								        D3DXMATRIX projectionMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColor, 
								        D3DXVECTOR4 diffuseColor, D3DXVECTOR3 cameraPosition, ID3D11ShaderResourceView* groundTexture,
								        ID3D11ShaderResourceView* wallTexture)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    uint32 bufferNumber;

	shaderMatrixBuffer_t* matrixBufferPtr;
	shaderCameraBuffer_t* cameraBufferPtr;
	ambientLightBuffer_t* lightBufferPtr;
	


	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	matrixBufferPtr = (shaderMatrixBuffer_t*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = worldMatrix;
	matrixBufferPtr->view = viewMatrix;
	matrixBufferPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finally set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // Lock the camera constant buffer so it can be written to.
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	cameraBufferPtr = (shaderCameraBuffer_t*)mappedResource.pData;

	// Copy the camera position into the constant buffer.
	cameraBufferPtr->cameraPosition = cameraPosition;
	cameraBufferPtr->padding = 0.0f;

	// Unlock the camera constant buffer.
	deviceContext->Unmap(m_cameraBuffer, 0);

	// Set the position of the camera constant buffer in the vertex shader.
	bufferNumber = 1;

	// Now set the camera constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);

    // Set shader texture resources in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &groundTexture);
    deviceContext->PSSetShaderResources(1, 1, &wallTexture);

    // Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	lightBufferPtr = (ambientLightBuffer_t*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	lightBufferPtr->ambientColor = ambientColor;
	lightBufferPtr->diffuseColor = diffuseColor;
	lightBufferPtr->lightDirection = lightDirection;
    lightBufferPtr->padding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	return true;
}


// -----------------------------------------------------------
// Shader SetParameters function for specular light shader
// -----------------------------------------------------------
bool Shader::SetSpecularShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
								         D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, 
								         D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 cameraPosition, D3DXVECTOR4 specularColor, 
								         float specularPower)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    uint32 bufferNumber;

	shaderMatrixBuffer_t* matrixBufferPtr;
	shaderCameraBuffer_t* cameraBufferPtr;
	specularLightBuffer_t* lightBufferPtr;
	


	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	matrixBufferPtr = (shaderMatrixBuffer_t*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = worldMatrix;
	matrixBufferPtr->view = viewMatrix;
	matrixBufferPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finally set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

    // Lock the camera constant buffer so it can be written to.
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	cameraBufferPtr = (shaderCameraBuffer_t*)mappedResource.pData;

	// Copy the camera position into the constant buffer.
	cameraBufferPtr->cameraPosition = cameraPosition;
	cameraBufferPtr->padding = 0.0f;

	// Unlock the camera constant buffer.
	deviceContext->Unmap(m_cameraBuffer, 0);

	// Set the position of the camera constant buffer in the vertex shader.
	bufferNumber = 1;

	// Now set the camera constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);

    // Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

    // Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	lightBufferPtr = (specularLightBuffer_t*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	lightBufferPtr->ambientColor = ambientColor;
	lightBufferPtr->diffuseColor = diffuseColor;
	lightBufferPtr->lightDirection = lightDirection;
	lightBufferPtr->specularPower = specularPower;
	lightBufferPtr->specularColor = specularColor;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer, 0);

	// Set the position of the light constant buffer in the pixel shader.
	bufferNumber = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	return true;
}


void Shader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

    // Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

} // end of namespace Gumshoe
