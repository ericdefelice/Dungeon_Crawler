/*!
  @file
  entity.cpp

  @brief
  Functionality needed for entities in the game.

  @detail
  Will hold the pointer to the entity model and will do movement/collision
  for the entity.  There are also many types of entities.
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "entity.h"
#include "model.cpp"


namespace Gumshoe {

Entity::Entity()
{
	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_position.z = 0.0f;

	m_rotation.x = 0.0f;
	m_rotation.y = 0.0f;
	m_rotation.z = 0.0f;

	m_velocity = {0.0f, 0.0f, 0.0f};
	m_accel = {0.0f, 0.0f, 0.0f};
	m_maxVelocity = JOG_SPEED;
    m_groundDragEn = true;

    m_Model = nullptr;
}


Entity::~Entity()
{
}


bool Entity::Init(ID3D11Device* device, LPCSTR* textureFilename, char* modelFilename, Vector3_t playerModelOffset)
{
    bool result;


    m_Model = new Model;
    if(!m_Model)
    {
        return false;
    }

    // Initialize the model for this entity
    result = m_Model->Init(device, textureFilename, modelFilename, playerModelOffset);
    if(!result)
    {
        return false;
    }

    return true;
}


void Entity::Shutdown()
{
    // Release the model texture.
    if (m_Model)
    {
        m_Model->Shutdown();
        delete m_Model;
        m_Model = nullptr;
    }

    return;
}


void Entity::Render(ID3D11DeviceContext* deviceContext)
{
    // Render the model for the entity
    m_Model->Render(deviceContext);

    return;
}


int Entity::GetIndexCount()
{
    return m_Model->GetIndexCount();
}


ID3D11ShaderResourceView* Entity::GetTexture()
{
    return m_Model->GetTexture();
}


void Entity::SetPosition(Vector3_t inPosition)
{
	m_position.x = inPosition.x;
	m_position.y = inPosition.y;
	m_position.z = inPosition.z;

	return;
}


void Entity::SetRotation(Vector3_t inRotation)
{
	m_rotation.x = inRotation.x;
	m_rotation.y = inRotation.y;
	m_rotation.z = inRotation.z;

	return;
}


void Entity::GetPosition(Vector3_t& outPosition)
{
	outPosition.x = m_position.x;
	outPosition.y = m_position.y;
	outPosition.z = m_position.z;

	return;
}


void Entity::GetRotation(Vector3_t& outRotation)
{
	outRotation.x = m_rotation.x;
	outRotation.y = m_rotation.y;
	outRotation.z = m_rotation.z;

	return;
}


void Entity::Move(ID3D11Device* device, uint32 moveBitmask, float frameTime)
{
    // CHANGE MOVE_BITMASK TO V3 inAccel
    float dt = frameTime/1000.0f;
    Vector3_t accelVec = {0, 0, 0};

    // Get the rotation in radians to correct in the movement
    float moveDirCos = 1.0f;
    float strafeDirCos = 0.0f;

    // Check input buttons and add acceleration to player
    if ((moveBitmask & 0x01)>>0)
    {
    	accelVec.x += (PLAYER_ACCEL*strafeDirCos);
        accelVec.z += (PLAYER_ACCEL*moveDirCos);
    }

    if ((moveBitmask & 0x02)>>1)
    {
    	accelVec.x -= (PLAYER_ACCEL*strafeDirCos);
        accelVec.z -= (PLAYER_ACCEL*moveDirCos);
    }

    if ((moveBitmask & 0x04)>>2)
    {
    	accelVec.x -= (PLAYER_ACCEL*moveDirCos);
        accelVec.z += (PLAYER_ACCEL*strafeDirCos);
    }

    if ((moveBitmask & 0x08)>>3)
    {
    	accelVec.x += (PLAYER_ACCEL*moveDirCos);
        accelVec.z -= (PLAYER_ACCEL*strafeDirCos);
    }

    // Normalize the X & Z movement
    float accelLength = LengthSq(accelVec);
    if(accelLength > 1.0f)
    {
        accelVec *= (1.0f / SquareRoot(accelLength));
    }

    // Twiddle factor for movment speed
    //float entitySpeed = 15.0f; // m/s^2
    float entitySpeed = WALK_SPEED*8.0f;
    accelVec *= entitySpeed;

    // TODO(ebd): ODE here!
    if (m_groundDragEn)
    {
        accelVec.x += -8.0f*m_velocity.x;
        accelVec.z += -8.0f*m_velocity.z;  
    }
    else
    {
        accelVec.x += -5.0f*m_velocity.x;
        accelVec.z += -5.0f*m_velocity.z;
    }

 
    // Now perform jump check
    if ((moveBitmask & 0x10)>>4)
    {
        accelVec.y += (20.0f*entitySpeed);
    }

    // Add gravity
    accelVec.y -= GRAVITY_ACCEL;


    // Now do physics movement
    Vector3_t oldPosition = m_position;
    Vector3_t positionDelta = (0.5f*accelVec*Square(dt) + m_velocity*dt);
    
    m_velocity = accelVec*dt + m_velocity;
    m_position = oldPosition + positionDelta;

    if (m_position.y < 0)
    {
        m_position.y = 0;
        m_velocity.y = 0;
    }

    m_Model->UpdatePosition(device, m_position);
}


void Entity::Rotate(Vector3_t inRotate, float frameTime)
{
/*
	if ((rotateBitmask & 0x01)>>0)
    {
    	m_rotation.y -= rotateSpeed;
    }

    if ((rotateBitmask & 0x02)>>1)
    {
    	m_rotation.y += rotateSpeed;
    }

    if ((rotateBitmask & 0x04)>>2)
    {
    	m_rotation.x -= rotateSpeed;
    }

    if ((rotateBitmask & 0x08)>>3)
    {
    	m_rotation.x += rotateSpeed;
    }
*/
    
    m_rotation.x += inRotate.x/2;
    m_rotation.y += inRotate.y/2;
    m_rotation.z += inRotate.z/2;
}


void Entity::SetOnGround(bool inOnGround)
{
    m_groundDragEn = inOnGround;
}

} // end of namespace Gumshoe
