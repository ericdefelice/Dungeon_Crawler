/*!
  @file
  entity.h

  @brief
  Functionality needed for entities in the game.

  @detail
  Will hold the pointer to the entity model and will do movement/collision
  for the entity.  There are also many types of entities.
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "gumshoe_math.h"
//#include "physics_aabb.h"
//#include "physics_movement.h"
#include "model.h"

// MOVEMENT SPEED
// 0.75 - 1.25 for crouching/walking
// 2.5 - 3 m/s for jogging
// 10 - 12 m/s for sprinting
const float WALK_SPEED = 1.0f;
const float JOG_SPEED  = 2.75f;
const float SPRINT_SPEED = 10.0f;

const float PLAYER_ACCEL = 1.0f;
const float JUMP_ACCEL = 0.5f;
const float GRAVITY_ACCEL = 9.8f;

// MOVEMENT EQUATIONS
// p_new = 1/2*a*t^2 + v*t + p_old
// v_new = a*t + v_old
// accel = CONSTANT_VAL (diff for each entity_type)

namespace Gumshoe {

//--------------------------------------------
// Entity class definition
//--------------------------------------------
class Entity
{
public:
	Entity();
	~Entity();

	bool Init(ID3D11Device*, LPCSTR*, char*, Vector3_t);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	void SetPosition(Vector3_t);
	void SetRotation(Vector3_t);
	void GetPosition(Vector3_t&);
	void GetRotation(Vector3_t&);

	void Move(ID3D11Device*, uint32, float);
	void Rotate(Vector3_t, float);

	void SetOnGround(bool);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	Vector3_t m_position, m_rotation;
	float m_maxVelocity;
	Vector3_t m_velocity, m_accel;
	bool m_groundDragEn;
	
	Model* m_Model;

    //Physics::Movement* m_Movement;
    //Physics::AABB* m_BoundingBox;
    //Physics::Collision* m_Collision;
};

} // end of namespace Gumshoe
