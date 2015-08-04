/*!
  @file
  camera.cpp

  @brief
  Functionality for the game camera.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "camera.h"
#include "physics_movement.cpp"


namespace Gumshoe {

Camera::Camera()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}


Camera::~Camera()
{
}


void Camera::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void Camera::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}


D3DXVECTOR3 Camera::GetPosition()
{
	return D3DXVECTOR3(m_positionX, m_positionY, m_positionZ);
}


void Camera::GetPosition(float& x, float& y, float& z)
{
	x = m_positionX;
	y = m_positionY;
	z = m_positionZ;

	return;
}


void Camera::GetRotation(float& x, float& y, float& z)
{
	x = m_rotationX;
	y = m_rotationY;
	z = m_rotationZ;

	return;
}


void Camera::Rotate(uint32 rotateBitmask, float frameTime)
{
	float rotateSpeed = 0.25f;

	if ((rotateBitmask & 0x01)>>0)
    {
    	m_rotationY -= rotateSpeed;
    }

    if ((rotateBitmask & 0x02)>>1)
    {
    	m_rotationY += rotateSpeed;
    }

    if ((rotateBitmask & 0x04)>>2)
    {
    	m_rotationX -= rotateSpeed;
    }

    if ((rotateBitmask & 0x08)>>3)
    {
    	m_rotationX += rotateSpeed;
    }
	//Physics::TurnLeft(((rotateBitmask & 0x04)>>2), m_rotateLeftSpeedX, m_rotationX, frameTime);
    //Physics::TurnRight(((rotateBitmask & 0x08)>>3), m_rotateRightSpeedX, m_rotationX, frameTime);
    //Physics::TurnLeft(((rotateBitmask & 0x01)>>0), m_rotateLeftSpeedY, m_rotationY, frameTime);
    //Physics::TurnRight(((rotateBitmask & 0x02)>>1), m_rotateRightSpeedY, m_rotationY, frameTime); 
}


void Camera::Move(uint32 moveBitmask, float frameTime)
{
    float moveSpeed = 0.25f;

    if ((moveBitmask & 0x01)>>0)
    {
    	m_positionZ += moveSpeed;
    }

    if ((moveBitmask & 0x02)>>1)
    {
    	m_positionZ -= moveSpeed;
    }

    if ((moveBitmask & 0x04)>>2)
    {
    	m_positionX -= moveSpeed;
    }

    if ((moveBitmask & 0x08)>>3)
    {
    	m_positionX += moveSpeed;
    }

    if ((moveBitmask & 0x10)>>4)
    {
    	m_positionY += moveSpeed;
    }

    if ((moveBitmask & 0x20)>>5)
    {
    	m_positionY -= moveSpeed;
    }

    //Physics::MoveLeft(((moveBitmask & 0x01)>>0), m_moveLeftSpeedX, m_positionX, frameTime);
    //Physics::MoveRight(((moveBitmask & 0x02)>>1), m_moveRightSpeedX, m_positionX, frameTime);
    //Physics::MoveLeft(((moveBitmask & 0x04)>>2), m_moveLeftSpeedY, m_positionY, frameTime);
    //Physics::MoveRight(((moveBitmask & 0x08)>>3), m_moveRightSpeedY, m_positionY, frameTime);
    //Physics::MoveLeft(((moveBitmask & 0x10)>>4), m_moveLeftSpeedZ, m_positionZ, frameTime);
    //Physics::MoveRight(((moveBitmask & 0x20)>>5), m_moveRightSpeedZ, m_positionZ, frameTime);
}


void Camera::Render()
{
	D3DXVECTOR3 up, position, lookAt;
	float yaw, pitch, roll;
	D3DXMATRIX rotationMatrix;


	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup the position of the camera in the world.
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	// Setup where the camera is looking by default.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	// conversion = PI radians / 180 degrees = 0.017...
	pitch = m_rotationX * 0.0174532925f;
	yaw   = m_rotationY * 0.0174532925f;
	roll  = m_rotationZ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, yaw, pitch, roll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	D3DXVec3TransformCoord(&lookAt, &lookAt, &rotationMatrix);
	D3DXVec3TransformCoord(&up, &up, &rotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = position + lookAt;

	// Finally create the view matrix from the three updated vectors.
	D3DXMatrixLookAtLH(&m_viewMatrix, &position, &lookAt, &up);

	return;
}


void Camera::GetViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}


void Camera::RenderBaseViewMatrix()
{
	D3DXVECTOR3 up, position, lookAt;
	float radians;


	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup the position of the camera in the world.
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	// Calculate the rotation in radians.
	radians = m_rotationY * 0.0174532925f;

	// Setup where the camera is looking.
	lookAt.x = sinf(radians) + m_positionX;
	lookAt.y = m_positionY;
	lookAt.z = cosf(radians) + m_positionZ;

	// Create the base view matrix from the three vectors.
	D3DXMatrixLookAtLH(&m_baseViewMatrix, &position, &lookAt, &up);

	return;
}


void Camera::GetBaseViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_baseViewMatrix;
	return;
}

} // end of namespace Gumshoe
