/*!
  @file
  frustum.h

  @brief
  Create a view frustum and check if objects are in it.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <d3dx10math.h>


namespace Gumshoe {

//--------------------------------------------
// Frustum class definition
//--------------------------------------------
class Frustum
{
public:
	Frustum();
	~Frustum();

	void ConstructFrustum(float, D3DXMATRIX, D3DXMATRIX);

	bool CheckPoint(float, float, float);
	bool CheckCube(float, float, float, float);
	bool CheckSphere(float, float, float, float);
	bool CheckRectangle(float, float, float, float, float, float);

private:
	D3DXPLANE m_planes[6];
};

} // end of namespace Gumshoe