/*!
  @file
  physics_movement.h

  @brief
  Part of the physics engine to control object movement.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "gumshoe_math.h"
#include <math.h>


namespace Gumshoe {

namespace Physics {

//--------------------------------------------
// Movement class definition
//--------------------------------------------
class Movement {
public:
	Movement();
	~Movement();

	void Move(Vector3_t*, Vector3_t, float);
    void Rotate(Vector3_t*, Vector3_t, float);

	void SetVelocity(float);
	void SetAngularVelocity(float);

	float GetVelocity();
	float GetAngularVelocity();

private:
	float m_linearAccel, m_angularAccel;
};

} // end of namespace Physics

} // end of namespace Gumshoe
