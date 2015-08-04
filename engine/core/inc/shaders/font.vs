/*!
  @file
  font.vs

  @brief
  Font vertex shader.

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
    float4 tex : TEXCOORD0;
};

struct pixelInput_t
{
    float4 position : SV_POSITION;
    float4 tex : TEXCOORD0;
};


//--------------------------------------------
// Vertex Shader Imlementation
//--------------------------------------------
pixelInput_t FontVertexShader(vertexInput_t input)
{
    pixelInput_t output;
    

	  // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	  // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	  // Store the texture coordinates for the pixel shader.
	  output.tex = input.tex;
    
    return output;
}