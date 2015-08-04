/*!
  @file
  camera.h

  @brief
  Functionality needed for the camera.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "gumshoe_math.h"
#include "physics_movement.h"
#include <d3dx10math.h>

// MOVEMENT SPEED
// 0.75 - 1.25 for crouching/walking
// 2.5 - 3 m/s for jogging
// 10 - 12 m/s for sprinting

// MOVEMENT EQUATIONS
// p_new = 1/2*a*t^2 + v*t + p_old
// v_new = a*t + v_old
// accel = CONSTANT_VAL (diff for each entity_type)

namespace Gumshoe {

//--------------------------------------------
// Camera class definition
//--------------------------------------------
class Camera
{
public:
	Camera();
	~Camera();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void GetPosition(float&, float&, float&);
	void GetRotation(float&, float&, float&);

    D3DXVECTOR3 GetPosition();	

	void Rotate(uint32, float);
	void Move(uint32, float);

	void Render();
	void GetViewMatrix(D3DXMATRIX&);

	void RenderBaseViewMatrix();
	void GetBaseViewMatrix(D3DXMATRIX&);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	D3DXMATRIX m_viewMatrix, m_baseViewMatrix;
};

} // end of namespace Gumshoe