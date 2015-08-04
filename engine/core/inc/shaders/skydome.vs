/*!
  @file
  skydome.vs

  @brief
  SkyDome vertex shader.

  @detail
*/


//--------------------------------------------
// Globals
//--------------------------------------------
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//--------------------------------------------
// Typedefs
//--------------------------------------------
struct vertexInput_t
{
    float4 position : POSITION;
};

struct pixelInput_t
{
    float4 position : SV_POSITION;
	float4 domePosition : TEXCOORD0;
};


//--------------------------------------------
// Vertex Shader Implementation
//--------------------------------------------
pixelInput_t SkyDomeVertexShader(vertexInput_t input)
{
    pixelInput_t output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Send the unmodified position through to the pixel shader.
	output.domePosition = input.position;

    return output;
}