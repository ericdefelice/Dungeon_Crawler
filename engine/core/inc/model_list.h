/*!
  @file
  model_list.h

  @brief
  Holds the list of all the models in the scene.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <d3dx10math.h>
#include <stdlib.h>
#include <time.h>


namespace Gumshoe {

//--------------------------------------------
// ModelList class definition
//--------------------------------------------
class ModelList
{
private:
	struct ModelInfo_t
	{
		D3DXVECTOR4 color;
		float positionX, positionY, positionZ;
	};

public:
	ModelList();
	~ModelList();

	bool Init(int);
	void Shutdown();

	int GetModelCount();
	void GetData(int, float&, float&, float&, D3DXVECTOR4&);

private:
	int m_modelCount;
	ModelInfo_t* m_ModelInfoList;
};

} // end of namespace Gumshoe
