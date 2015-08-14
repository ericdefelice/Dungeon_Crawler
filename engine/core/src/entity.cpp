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
    m_aabb = {0.0f, 0.0f, 0.0f};
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

    // Hardcode the entities model size for now (this is okay since there is only the player model for now)
    // There will need to be an API for the Collider and this will be set there in the future
    m_aabb.x = 0.5f;
    m_aabb.y = 1.83f;
    m_aabb.z = 0.25f;

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


bool Entity::TestFloorCollision(Vector3_t oldPos, Vector3_t posDelta, float *tMin)
{
    bool hit = false;
    float tEpsilon = 0.00001f;
    
    // Get the final postion based on the old position and the delta
    float newYPos = oldPos.y + posDelta.y;

    if (newYPos < 0.0f)
    {
        // Now find where along that vector the collision was
        float tResult = (-oldPos.y) / posDelta.y;
        
        // Only use the result if this collision is closer than another one
        if((tResult >= 0.0f) && (*tMin > tResult))
        {
            *tMin = Maximum(0.0f, tResult - tEpsilon);
            hit = true;
        }
    }

    return hit;
}


void Entity::Move(ID3D11Device* device, Vector3_t moveAccel, GameWorld* gameWorld, float frameTime)
{
    // CHANGE MOVE_BITMASK TO V3 inAccel
    float dt = frameTime/1000.0f;
    Vector3_t accelVec = {0, 0, 0};

    // Get the rotation in radians to correct in the movement
    float moveDirCos = 1.0f;
    float strafeDirCos = 0.0f;

    // Set the acceleration vector to just the x & z components first for normalization purposes
    accelVec.x = moveAccel.x;
    accelVec.z = moveAccel.z;

    // Normalize the X & Z movement
    float accelLength = LengthSq(accelVec);
    if(accelLength > 1.0f)
    {
        accelVec *= (1.0f / SquareRoot(accelLength));
    }

    // Twiddle factor for movment speed (8.0 is used to combat the drag for movement)
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

    // Now add in the y acceleration value
    accelVec.y = moveAccel.y*entitySpeed*20.0f;

    // Add gravity
    accelVec.y -= GRAVITY_ACCEL;

    // Now do physics based movement
    
    // Need to add Physics objects to the entity
    // RigidBody object -> holds postion, rotation, velocity, accel, and handles movement for the entity
    // Collider object -> holds information about the bounding box and handles collision functions
    // m_RigidBody->Move(&oldPosition);
    // m_Collider->CollisionTest(&position, &rotation);  // This should return a bool for if there was a collision or not
    
    // Do the movement internally for now
    Vector3_t oldPosition = m_position;
    Vector3_t positionDelta = (0.5f*accelVec*Square(dt) + m_velocity*dt);
    
    m_velocity = accelVec*dt + m_velocity;
    m_position = oldPosition + positionDelta;

    // Do minkowski based collision here
    // First get a list of game map tiles to search through
/*
    uint32 minTileX = FloorFloattoUint32(Minimum(m_position.x, oldPosition.x) - m_aabb.x/2);
    uint32 maxTileX = FloorFloattoUint32(Maximum(m_position.x, oldPosition.x) + m_aabb.x/2);
    uint32 minTileZ = FloorFloattoUint32(Minimum(m_position.z, oldPosition.z) - m_aabb.z/2);
    uint32 maxTileZ = FloorFloattoUint32(Maximum(m_position.z, oldPosition.z) + m_aabb.z/2);
*/
    uint32 minTileX = FloorFloattoUint32(Minimum(m_position.x, oldPosition.x));
    uint32 maxTileX = FloorFloattoUint32(Maximum(m_position.x, oldPosition.x));
    uint32 minTileZ = FloorFloattoUint32(Minimum(m_position.z, oldPosition.z));
    uint32 maxTileZ = FloorFloattoUint32(Maximum(m_position.z, oldPosition.z));

    // Set the time remaining on the search to 1.0
    // We use this to find where the collision was along the movement vector
    float tMin = 1.0f;
    Vector3_t collisionNormal = {0.0f, 0.0f, 0.0f};
    bool floorCollision = false;

    // Test each tile in the search space
    for (uint32 tileX = minTileX; tileX <= maxTileX; tileX++)
    {
        for (uint32 tileZ = minTileZ; tileZ <= maxTileZ; tileZ++)
        {
            // Get the normal for the current tile (is it a wall or floor or neither)
            collisionNormal = gameWorld->GetTileNormal(tileX, tileZ);

            // Get the current tile value, and if there is a valid tile then do collision there
            if (collisionNormal.y == 1.0f)
            {
                if (TestFloorCollision(oldPosition, positionDelta, &tMin))
                {
                    collisionNormal = {0.0f, 1.0f, 0.0f};
                    floorCollision = true;
                }     
            } 
        }
    }

    if (!floorCollision)
        collisionNormal = {0.0f, 0.0f, 0.0f};

    // Now update the entity position based on the output of the collision detection
    // Update the timeRemaining using tMin
    m_position.y = oldPosition.y + tMin*positionDelta.y;
    m_velocity = m_velocity - 1*Inner(m_velocity, collisionNormal)*collisionNormal;

/*
    if (m_position.y < 0)
    {
        m_position.y = 0;
        m_velocity.y = 0;
    }
*/

    // Now updat the model position to be where the entity is
    m_Model->UpdatePosition(device, m_position);
}


void Entity::Rotate(Vector3_t inRotate, float frameTime)
{
    m_rotation.x += inRotate.x/2;
    m_rotation.y += inRotate.y/2;
    m_rotation.z += inRotate.z/2;
}


void Entity::SetOnGround(bool inOnGround)
{
    m_groundDragEn = inOnGround;
}

} // end of namespace Gumshoe
