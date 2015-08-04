/*!
  @file
  quadtree.h

  @brief
  Functionality for the quad tree in the Engine.

  @detail
  Quad trees are used to determine which areas of the world are viewable.
*/

#pragma once

//--------------------------------------------
// Globals
//--------------------------------------------
const int MAX_TRIANGLES = 10000;

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "gumshoe_math.h"
#include "escape_world.h"
#include "frustum.h"
#include "shader.h"

namespace Gumshoe {

//--------------------------------------------
// QuadTree class definition
//--------------------------------------------
class QuadTree
{
private:
	struct vertex_t
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 texture;
		D3DXVECTOR3 normal;
		D3DXVECTOR4 color;
	};

	struct node_t
	{
        float positionX, positionZ, width;
		int triangleCount;
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		Vector3_t* vertexArray;
        node_t* nodes[4];
	};

public:
	QuadTree();
	~QuadTree();

	bool Init(GameWorld*, ID3D11Device*);
	void Shutdown();
	void Render(Frustum*, ID3D11DeviceContext*, Shader*);

	int GetDrawCount();
	bool GetHeightAtPosition(float, float, float&);

private:
	void CalculateMeshDimensions(int, float&, float&, float&);
	void CreateTreeNode(node_t*, float, float, float, ID3D11Device*);
	int CountTriangles(float, float, float);
	bool IsTriangleContained(int, float, float, float);
	void ReleaseNode(node_t*);
	void RenderNode(node_t*, Frustum*, ID3D11DeviceContext*, Shader*);

	void FindNode(node_t*, float, float, float&);
	bool CheckHeightOfTriangle(float, float, float&, float[3], float[3], float[3]);

private:
	int m_triangleCount, m_drawCount;
	vertex_t* m_vertexList;
	node_t* m_rootNode;
};

} // end of namespace Gumshoe
