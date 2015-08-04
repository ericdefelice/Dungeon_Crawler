/*!
  @file
  model_list.cpp

  @brief
  Holds the list of all the models in the scene.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "model_list.h"


namespace Gumshoe {

ModelList::ModelList()
{
	m_ModelInfoList = nullptr;
}


ModelList::~ModelList()
{
}


bool ModelList::Init(int numModels)
{
	int i;
	float red, green, blue;


	// Store the number of models.
	m_modelCount = numModels;

	// Create a list array of the model information.
	m_ModelInfoList = new ModelInfo_t[m_modelCount];
	if(!m_ModelInfoList)
	{
		return false;
	}

	// Seed the random generator with the current time.
	srand((unsigned int)time(NULL));

	// Go through all the models and randomly generate the model color and position.
	for(i=0; i<m_modelCount; i++)
	{
		// Generate a random color for the model.
		red = (float)rand() / RAND_MAX;
		green = (float)rand() / RAND_MAX;
		blue = (float)rand() / RAND_MAX;

		m_ModelInfoList[i].color = D3DXVECTOR4(red, green, blue, 1.0f);

		// Generate a random position in front of the viewer for the mode.
		m_ModelInfoList[i].positionX = (((float)rand()-(float)rand())/RAND_MAX) * 10.0f;
		m_ModelInfoList[i].positionY = (((float)rand()-(float)rand())/RAND_MAX) * 10.0f;
		m_ModelInfoList[i].positionZ = ((((float)rand()-(float)rand())/RAND_MAX) * 10.0f) + 5.0f;
	}

	return true;
}


void ModelList::Shutdown()
{
	// Release the model information list.
	if(m_ModelInfoList)
	{
		delete [] m_ModelInfoList;
		m_ModelInfoList = 0;
	}

	return;
}


int ModelList::GetModelCount()
{
	return m_modelCount;
}


void ModelList::GetData(int id, float& positionX, float& positionY, float& positionZ, D3DXVECTOR4& color)
{
	positionX = m_ModelInfoList[id].positionX;
	positionY = m_ModelInfoList[id].positionY;
	positionZ = m_ModelInfoList[id].positionZ;

	color = m_ModelInfoList[id].color;

	return;
}

} // end of namespace Gumshoe