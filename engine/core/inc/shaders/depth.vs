/*!
  @file
  depth.vs

  @brief
  Depth vertex shader.

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
    float4 depthPosition : TEXCOORD0;
};


//--------------------------------------------
// Vertex Shader Imlementation
//--------------------------------------------
pixelInput_t DepthVertexShader(vertexInput_t input)
{
    pixelInput_t output;
    

	  // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	  // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	  // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
    
    return output;
}