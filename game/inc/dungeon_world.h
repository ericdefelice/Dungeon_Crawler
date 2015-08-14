/*!
  @file
  dungeon_world.h

  @brief
  Object to create and hold the game world.

  @detail
*/

#pragma once

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include "gumshoe_math.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <random>
#include <vector>
#include <fstream>
#include <cmath>
#include "texture.h"

//--------------------------------------------
// Globals
//--------------------------------------------
const int TEXTURE_REPEAT = 2;


//--------------------------------------------
// GameWorld class definition
//--------------------------------------------
class GameWorld
{
public:
	enum Tile
	{
		Unused		= ' ',
		Floor		= '.',
		Corridor	= ',',
		Wall		= '|',
		ClosedDoor	= '+',
		OpenDoor	= '-',
		UpStairs	= '<',
		DownStairs	= '>'
	};
 
	enum Direction
	{
		North,
		South,
		West,
		East,
		DirectionCount
	};

	enum TileFeatures
	{
        NorthWestFloor  = (1<<0),
        NorthEastFloor  = (1<<1),
        SouthWestFloor  = (1<<2),
        SouthEastFloor  = (1<<3),
        NorthWall       = (1<<4),
        SouthWall       = (1<<5),
        EastWall        = (1<<6),
        WestWall        = (1<<7),
        NorthWallCap    = (1<<8),
        SouthWallCap    = (1<<9),
        EastWallCap     = (1<<10),
        WestWallCap     = (1<<11),
        VertDoorway     = (1<<12),
        HorizDoorway    = (1<<13)
	};

private:
	struct gameWorldVertex_t
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	    D3DXVECTOR3 normal;
	    D3DXVECTOR4 color;
	};

	struct gameWorldGrid_t 
	{ 
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
		float r, g, b;
		uint32 geoFeatures;
		//int mat_r, mat_g, mat_b;
	};

	struct materialGroup_t 
	{ 
		int textureIndex1, textureIndex2, alphaIndex;
		int r, g, b;
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount;
		gameWorldVertex_t* vertices;
		unsigned long* indices;
	};

	struct Rect
    {
	    int x, y;
	    int xSize, ySize;
    };

public:
	GameWorld();
	~GameWorld();

	bool Init(ID3D11Device*, LPCSTR*, LPCSTR*);
	void Shutdown();

	void Render(ID3D11DeviceContext*);
    // Render function for using material groups
	//bool Render(ID3D11DeviceContext*, Gumshoe::Shader*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3);

	int GetIndexCount();
	//ID3D11ShaderResourceView* GetTexture();
	//ID3D11ShaderResourceView* GetDetailMapTexture();

	ID3D11ShaderResourceView* GetGroundTexture();
	ID3D11ShaderResourceView* GetWallTexture();
	
	//ID3D11ShaderResourceView* GetTexture(uint32);
	//ID3D11ShaderResourceView* GetNormal();

	int GetVertexCount();
	void CopyVertexArray(void*);

	void GetWorldSize(int&, int&);
	void GetUpStairsLocation(float&, float&);

	Gumshoe::Vector3_t GetTileNormal(int, int);

private:
	bool LoadHeightMap(char*);
	void ScaleHeightMap();
	bool CalculateNormals();

	bool GenerateWorld(uint32, int);
	void ReleaseWorldGrid();

	// Procedural generation functions
	bool GenerateRandomWorld(int);
	void PrintWorld();
	char GetTile(int, int);
	void SetTile(int, int, char);
	bool CreateFeature();
	bool CreateFeature(int, int, Direction);
	bool MakeRoom(int, int, Direction, bool);
	bool MakeCorridor(int, int, Direction);
	bool PlaceRect(const Rect&, char);
	bool PlaceObject(char);

	void AddTileFloorGeometry(uint32, std::vector<gameWorldVertex_t>&, std::vector<unsigned long>&, uint32&);
	void AddTileWallGeometry(uint32, std::vector<gameWorldVertex_t>&, std::vector<unsigned long>&, uint32&);
	void AddTileWallCapGeometry(uint32, std::vector<gameWorldVertex_t>&, std::vector<unsigned long>&, uint32&);
	void AddTileDoorwayGeometry(uint32, std::vector<gameWorldVertex_t>&, std::vector<unsigned long>&, uint32&);
	
    void CalculateTextureCoordinates();
	bool LoadTextures(ID3D11Device*, LPCSTR*, LPCSTR*);
	void ReleaseTextures();

	bool LoadColorMap(char*);

	//bool LoadMaterialFile(char*, char*, ID3D11Device*);
	//bool LoadMaterialMap(char*);
	//bool LoadMaterialBuffers(ID3D11Device*);
	//void ReleaseMaterials();

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	
private:
	uint32 m_worldLength, m_worldWidth, m_worldHeight;
	int m_vertexCount, m_indexCount;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	gameWorldGrid_t* m_gameWorldGrid;
	//Gumshoe::Texture *m_Texture, *m_DetailTexture;
    Gumshoe::Texture *m_GroundTexture, *m_WallTexture;

	std::vector<gameWorldVertex_t> m_vertices;

	// Procedural generation variables
	int m_procWorldLength, m_procWorldWidth;
	uint32 m_tileCount;
	std::vector<char> m_tiles;
	std::vector<Rect> m_rooms; // rooms to put items, stairs, enemies, etc.
	std::vector<Rect> m_exits; // could be an exit on any of the 4 room sides

	//uint32 m_textureCount, m_materialCount;
};
