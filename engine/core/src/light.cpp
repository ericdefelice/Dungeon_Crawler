/*!
  @file
  light.cpp

  @brief
  Functionality for lights in the engine.

  @detail
  Maintains the direction and the color for the light (and type of light in the future).
  Each light in the world will need an instantiation of this class 
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "light.h"


namespace Gumshoe {

Light::Light()
{
}


Light::~Light()
{
}


void Light::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = D3DXVECTOR4(red, green, blue, alpha);
	return;
}


void Light::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = D3DXVECTOR4(red, green, blue, alpha);
	return;
}


void Light::SetDirection(float x, float y, float z)
{
	m_direction = D3DXVECTOR3(x, y, z);
	return;
}


void Light::SetSpecularColor(float red, float green, float blue, float alpha)
{
	m_specularColor = D3DXVECTOR4(red, green, blue, alpha);
	return;
}


void Light::SetSpecularPower(float power)
{
	m_specularPower = power;
	return;
}


D3DXVECTOR4 Light::GetAmbientColor()
{
	return m_ambientColor;
}


D3DXVECTOR4 Light::GetDiffuseColor()
{
	return m_diffuseColor;
}


D3DXVECTOR3 Light::GetDirection()
{
	return m_direction;
}


D3DXVECTOR4 Light::GetSpecularColor()
{
	return m_specularColor;
}


float Light::GetSpecularPower()
{
	return m_specularPower;
}


} // end of namespace Gumshoe
