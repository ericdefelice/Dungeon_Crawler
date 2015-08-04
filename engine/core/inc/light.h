/*!
  @file
  light.h

  @brief
  Functionality for lights in the engine.

  @detail
  Maintains the direction and the color for the light (and type of light in the future).
  Each light in the world will need an instantiation of this class 
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include <d3dx10math.h>


namespace Gumshoe {

//--------------------------------------------
// Light class definition
//--------------------------------------------
class Light
{
public:
	Light();
	~Light();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetDirection(float, float, float);
	void SetSpecularColor(float, float, float, float);
	void SetSpecularPower(float);

	D3DXVECTOR4 GetAmbientColor();
	D3DXVECTOR4 GetDiffuseColor();
	D3DXVECTOR3 GetDirection();
	D3DXVECTOR4 GetSpecularColor();
	float GetSpecularPower();

private:
	D3DXVECTOR4 m_ambientColor;
	D3DXVECTOR4 m_diffuseColor;
	D3DXVECTOR3 m_direction;
	D3DXVECTOR4 m_specularColor;
	float m_specularPower;
};

} // end of namespace Gumshoe
