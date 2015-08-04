/*!
  @file
  physics_aabb.h

  @brief
  Part of the physics engine to create bounding-boxes.

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
// AABB class definition
//--------------------------------------------
class AABB {
public:
	AABB();
	~AABB();

	void SetType(uint32);
    void SetPosition(Vector3_t*);
    void SetHeight(float);
	void SetRadius(float);

	uint32 GetType();
    Vector3_t GetPosition();
    float GetHeight();
	float GetRadius();

private:
	Vector3_t m_position;
	float m_height, m_radius;
	uint32 m_aabbType;
};

} // end of namespace Physics

} // end of namespace Gumshoe
