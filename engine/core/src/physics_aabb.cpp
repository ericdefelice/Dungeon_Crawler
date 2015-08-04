/*!
  @file
  physics_aabb.cpp

  @brief
  Part of the physics engine to create bounding-boxes.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "physics_aabb.h"


namespace Gumshoe {

namespace Physics {


AABB::AABB()
{
	m_linearAccel = 0.15f;
	m_angularAccel = 0.15f;
}


AABB::~AABB()
{
}

/*
void Movement::Move(Vec3* position, Vec3 desiredPosition, float frameTime)
{
	return;
}


void Movement::Rotate(Vec3* rotation, Vec3 desiredRotation, float frameTime)
{
	return;
}
*/

} // end of namespace Physics

} // end of namespace Gumshoe
