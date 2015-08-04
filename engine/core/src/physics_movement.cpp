/*!
  @file
  physics_movement.cpp

  @brief
  Part of the physics engine to control object movement.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "physics_movement.h"


namespace Gumshoe {

namespace Physics {


Movement::Movement()
{
	m_linearAccel = 0.15f;
	m_angularAccel = 0.15f;
}


Movement::~Movement()
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
