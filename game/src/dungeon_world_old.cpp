/*!
  @file
  dungeon_world.cpp

  @brief
  Object to create and hold the game world.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "dungeon_world.h"


GameWorld::GameWorld()
{
	m_gameWorldGrid = nullptr;
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_GroundTexture = nullptr;
	m_WallTexture = nullptr;

	//m_vertices = nullptr;

	//m_tiles = nullptr;
	//m_rooms = nullptr;
	//m_exits = nullptr;

	//m_Textures = nullptr;
	//m_Materials = nullptr;
}


GameWorld::~GameWorld()
{
}


bool GameWorld::Init(ID3D11Device* device, LPCSTR* groundTextureFilename, LPCSTR* wallTextureFilename)
{
	bool result;


	// Manually set the height of the game world. There is only 1 level for now
	m_worldHeight = 1;

    int maxFeatures = 60;

    // Generate the dungeon
	result = GenerateWorld(m_worldHeight, maxFeatures);
	if(!result)
	{
		return false;
	}

	PrintWorld();
/*
    // Calculate the normals for the world data.
	result = CalculateNormals();
	if(!result)
	{
		return false;
	}
*/
	// Calculate the texture coordinates.
	//CalculateTextureCoordinates();

	// Load the textures.
	result = LoadTextures(device, groundTextureFilename, wallTextureFilename);
	if(!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	return true;
}


void GameWorld::Shutdown()
{
	// Release the materials for the game world.
	//ReleaseMaterials();

	// Release the textures.
	ReleaseTextures();

	// Release the vertex and index buffer.
	ShutdownBuffers();

	// Release the game world itself.
	ReleaseWorldGrid();

	return;
}


void GameWorld::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


/*
ID3D11ShaderResourceView* GameWorld::GetTexture(uint32 index)
{
	return m_Textures[index].GetTexture();
}
*/


int GameWorld::GetIndexCount()
{
	return m_indexCount;
}

/*
ID3D11ShaderResourceView* GameWorld::GetTexture()
{
	return m_Texture->GetTexture();
}


ID3D11ShaderResourceView* GameWorld::GetDetailMapTexture()
{
	return m_DetailTexture->GetTexture();
}
*/

ID3D11ShaderResourceView* GameWorld::GetGroundTexture()
{
	return m_GroundTexture->GetTexture();
}


ID3D11ShaderResourceView* GameWorld::GetWallTexture()
{
	return m_WallTexture->GetTexture();
}


/*
int GameWorld::GetVertexCount()
{
	uint32 i;
	int vertexCount = 0;

    // Loop through all the materials and count the vertices for each
	for(i=0; i<m_materialCount; i++)
	{
		vertexCount += m_Materials[i].vertexCount;
	}

	return vertexCount;
}


void GameWorld::CopyVertexArray(void* vertexList)
{
	uint32 i;
	int vertexCount = 0;
    gameWorldVertex_t* vertexPtr = (gameWorldVertex_t*)vertexList;

    // Loop through all the materials and copy the vertices to the destination
	for(i=0; i<m_materialCount; i++)
	{
		vertexCount = m_Materials[i].vertexCount;
		if (vertexCount > 0)
		{
        	memcpy(vertexPtr, m_Materials[i].vertices, sizeof(gameWorldVertex_t) * vertexCount);
		    vertexPtr += vertexCount;
		}
	}

	return;
}
*/

int GameWorld::GetVertexCount()
{
	return m_vertexCount;
}


void GameWorld::CopyVertexArray(void* vertexList)
{
	//memcpy(vertexList, m_vertices, sizeof(gameWorldVertex_t) * m_vertexCount);

	return;
}


void GameWorld::GetWorldSize(int& length, int& width)
{
	// Return the length and width of the world.
	length = m_worldLength;
	width = m_worldWidth;

	return;
}


void GameWorld::GetUpStairsLocation(float& xPos, float& zPos)
{
	for (int y = 0; y < m_procWorldWidth; y++)
	{
		for (int x = 0; x < m_procWorldLength; x++)
		{
			if (GetTile(x, y) == UpStairs)
			{
				xPos = (float)x;
				zPos = (float)y;

				return;
			}
		}
	}
}


bool GameWorld::LoadHeightMap(char* filename)
{
	FILE* filePtr;
	int error;
	size_t count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;


	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_worldLength = bitmapInfoHeader.biWidth;
	m_worldWidth = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_worldLength * m_worldWidth * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	m_gameWorldGrid = new gameWorldGrid_t[m_worldLength * m_worldWidth];
	if(!m_gameWorldGrid)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k=0;

	// Read the image data into the height map.
	for(j=0; j<m_worldLength; j++)
	{
		for(i=0; i<m_worldWidth; i++)
		{
			height = bitmapImage[k];
			
			index = (m_worldWidth * j) + i;

			m_gameWorldGrid[index].x = (float)i;
			m_gameWorldGrid[index].y = (float)height;
			m_gameWorldGrid[index].z = (float)j;

			k+=3;
		}
	}

	// Release the bitmap image data.
	delete [] bitmapImage;
	bitmapImage = nullptr;

	return true;
}


void GameWorld::ScaleHeightMap()
{
	unsigned int i, j;


	for(j=0; j<m_worldLength; j++)
	{
		for(i=0; i<m_worldWidth; i++)
		{
			m_gameWorldGrid[(m_worldWidth * j) + i].y /= 15.0f;
		}
	}

	return;
}


bool GameWorld::CalculateNormals()
{
	uint32 i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	Vector3_t* normals;


	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new Vector3_t[(m_worldLength-1) * (m_worldWidth-1)];
	if(!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for(j=0; j<(m_worldLength-1); j++)
	{
		for(i=0; i<(m_worldWidth-1); i++)
		{
			index1 = (j * m_worldWidth) + i;
			index2 = (j * m_worldWidth) + (i+1);
			index3 = ((j+1) * m_worldWidth) + i;

			// Get three vertices from the face.
			vertex1[0] = m_gameWorldGrid[index1].x;
			vertex1[1] = m_gameWorldGrid[index1].y;
			vertex1[2] = m_gameWorldGrid[index1].z;
		
			vertex2[0] = m_gameWorldGrid[index2].x;
			vertex2[1] = m_gameWorldGrid[index2].y;
			vertex2[2] = m_gameWorldGrid[index2].z;
		
			vertex3[0] = m_gameWorldGrid[index3].x;
			vertex3[1] = m_gameWorldGrid[index3].y;
			vertex3[2] = m_gameWorldGrid[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_worldWidth-1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for(j=0; j<m_worldLength; j++)
	{
		for(i=0; i<m_worldWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if((i >= 1) && (j >= 1))
			{
				index = ((j-1) * (m_worldWidth-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if((i < (m_worldWidth-1)) && (j >= 1))
			{
				index = ((j-1) * (m_worldWidth-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if((i >= 1) && (j < (m_worldLength-1)))
			{
				index = (j * (m_worldWidth-1)) + (i-1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if((i < (m_worldWidth-1)) && (j < (m_worldLength-1)))
			{
				index = (j * (m_worldWidth-1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}
			
			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));
			
			// Get an index to the vertex location in the height map array.
			index = (j * m_worldWidth) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_gameWorldGrid[index].nx = (sum[0] / length);
			m_gameWorldGrid[index].ny = (sum[1] / length);
			m_gameWorldGrid[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete [] normals;
	normals = nullptr;

	return true;
}


bool GameWorld::GenerateWorld(uint32 worldFloors, int maxFeatures)
{
	//uint32 i, j;
	uint32 index;

    m_worldLength = 96;
    m_worldWidth = 96;
	
	// First generate the random dungeon
	bool result = GenerateRandomWorld(maxFeatures);
	if (!result)
	{
		return false;
	}

	// Create the structure to hold the game world data.
	m_gameWorldGrid = new gameWorldGrid_t[m_tileCount];
	if(!m_gameWorldGrid)
	{
		return false;
	}

	// Generate the game world geometry from the tilemap
	index = 0;
	for (int y = 0; y < m_procWorldWidth; ++y)
	{
	  for (int x = 0; x < m_procWorldLength; ++x)
	  {
		char tile = GetTile(x, y);
		
        // If the tile is used, fill the grid and increment the index
		if (tile != '.')
		{
		  m_gameWorldGrid[index].x = (float)x;
          m_gameWorldGrid[index].y = 0.0f;
          m_gameWorldGrid[index].z = (float)y;

          m_gameWorldGrid[index].tu = 0.0f;
		  m_gameWorldGrid[index].tv = 1.0f;

		  m_gameWorldGrid[index].nx = 0.0f;
		  m_gameWorldGrid[index].ny = 0.0f;
		  m_gameWorldGrid[index].nz = 0.0f;

		  m_gameWorldGrid[index].r = 0.0f;
		  m_gameWorldGrid[index].g = 0.0f;
		  m_gameWorldGrid[index].b = 0.0f;

		  m_gameWorldGrid[index].geoFeatures = 0;

		  if (tile == Wall)
          {
          	// Check to see which way the wall is facing to add normals
          	char northTile = GetTile(x, y+1);
          	char southTile = GetTile(x, y-1);
          	char eastTile  = GetTile(x+1, y);
          	char westTile  = GetTile(x-1, y);
          	char northEastTile = GetTile(x+1, y+1);
          	char southEastTile = GetTile(x+1, y-1);
          	char northWestTile = GetTile(x-1, y+1);
          	char southWestTile = GetTile(x-1, y-1);

          	// Check each adjacent tile to set the current tile features
            if (northWestTile == ' ' || northWestTile == ClosedDoor)
            {
                m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
            }
            
            if (northEastTile == ' ' || northEastTile == ClosedDoor)
            {
                m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
            }

            if (southWestTile == ' ' || southWestTile == ClosedDoor)
            {
                m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
            }

            if (southEastTile == ' ' || southEastTile == ClosedDoor)
            {
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
            }

            // Check the North tile
            if (northTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
            }
            else if (northTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthWall;
                m_gameWorldGrid[index].geoFeatures |= NorthWallCap;
            }
            else if (northTile == Wall)
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthWall;
            }

            // Check the South tile
            if (southTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
            }
            else if (southTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthWall;
                m_gameWorldGrid[index].geoFeatures |= SouthWallCap;
            }
            else if (southTile == Wall)
            {
            	m_gameWorldGrid[index].geoFeatures |= SouthWall;
            }

          	// Check the East tile
            if (eastTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
            }
            else if (eastTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= EastWall;
                m_gameWorldGrid[index].geoFeatures |= EastWallCap;
            }
            else if (eastTile == Wall)
            {
            	m_gameWorldGrid[index].geoFeatures |= EastWall;
            }

            // Check the West tile
            if (westTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
            }
            else if (westTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= WestWall;
                m_gameWorldGrid[index].geoFeatures |= WestWallCap;
            }
            else if (westTile == Wall)
            {
            	m_gameWorldGrid[index].geoFeatures |= WestWall;
            }

/*
          	// Check which way the wall is facing by looking at adjacent tiles
          	// Set the z-direction true normal
          	if (northTile == ' ' && southTile == ' ')
          	{
          		m_gameWorldGrid[index].nz = 2.0f;
          	}
          	else if (southTile == ' ')
          	{
          		m_gameWorldGrid[index].nz = -1.0f;
          	}
          	else if (northTile == ' ')
          	{
          		m_gameWorldGrid[index].nz = 1.0f;
          	}
          	else
          	{
          		m_gameWorldGrid[index].nz = 0.0f;
          	}
          	
          	// Set the x-direction true normal	
          	if (eastTile == ' ' && westTile == ' ')
          	{
          		m_gameWorldGrid[index].nx = 2.0f;
          	}
          	else if (westTile == ' ')
          	{
          		m_gameWorldGrid[index].nx = -1.0f;
          	}
          	else if (eastTile == ' ')
          	{
          		m_gameWorldGrid[index].nx = 1.0f;
          	}
          	else
          	{
          		m_gameWorldGrid[index].nx = 0.0f;
          	}
            
            // Now check if any half-walls are needed for the tile that have full walls
            if (m_gameWorldGrid[index].nx == 1.0f && m_gameWorldGrid[index].nz == 0.0f)
            {
                if (northWestTile == ' ' && southWestTile == ' ')
                {
                	m_gameWorldGrid[index].r = -1.0f;
                	m_gameWorldGrid[index].g = -1.0f;
                	m_gameWorldGrid[index].b = 2.0f;
                }
                else if (northWestTile == ' ')
                {
                    m_gameWorldGrid[index].r = -1.0f;
                	m_gameWorldGrid[index].g = -1.0f;
                	m_gameWorldGrid[index].b = 1.0f;
                }
                else if (southWestTile == ' ')
                {
                    m_gameWorldGrid[index].r = -1.0f;
                	m_gameWorldGrid[index].g = -1.0f;
                	m_gameWorldGrid[index].b = -1.0f;
                }
            }
            else if (m_gameWorldGrid[index].nx == -1.0f && m_gameWorldGrid[index].nz == 0.0f)
            {
                if (northEastTile == ' ' && southEastTile == ' ')
                {
                	m_gameWorldGrid[index].r = 1.0f;
                	m_gameWorldGrid[index].g = -1.0f;
                	m_gameWorldGrid[index].b = 2.0f;
                }
                else if (northEastTile == ' ')
                {
                    m_gameWorldGrid[index].r = 1.0f;
                	m_gameWorldGrid[index].g = -1.0f;
                	m_gameWorldGrid[index].b = 1.0f;
                }
                else if (southEastTile == ' ')
                {
                    m_gameWorldGrid[index].r = 1.0f;
                	m_gameWorldGrid[index].g = -1.0f;
                	m_gameWorldGrid[index].b = -1.0f;
                }
            }

            if (m_gameWorldGrid[index].nx == 0.0f && m_gameWorldGrid[index].nz == 1.0f)
            {
                if (southWestTile == ' ' && southEastTile == ' ')
                {
                	m_gameWorldGrid[index].r = 2.0f;
                	m_gameWorldGrid[index].g = 1.0f;
                	m_gameWorldGrid[index].b = -1.0f;
                }
                else if (southWestTile == ' ')
                {
                    m_gameWorldGrid[index].r = -1.0f;
                	m_gameWorldGrid[index].g = 1.0f;
                	m_gameWorldGrid[index].b = -1.0f;
                }
                else if (southEastTile == ' ')
                {
                    m_gameWorldGrid[index].r = 1.0f;
                	m_gameWorldGrid[index].g = 1.0f;
                	m_gameWorldGrid[index].b = -1.0f;
                }   
            }
            else if (m_gameWorldGrid[index].nx == 0.0f && m_gameWorldGrid[index].nz == -1.0f)
            {
                if (northWestTile == ' ' && northEastTile == ' ')
                {
                	m_gameWorldGrid[index].r = 2.0f;
                	m_gameWorldGrid[index].g = 1.0f;
                	m_gameWorldGrid[index].b = 1.0f;
                }
                else if (northWestTile == ' ')
                {
                    m_gameWorldGrid[index].r = -1.0f;
                	m_gameWorldGrid[index].g = 1.0f;
                	m_gameWorldGrid[index].b = 1.0f;
                }
                else if (northEastTile == ' ')
                {
                    m_gameWorldGrid[index].r = 1.0f;
                	m_gameWorldGrid[index].g = 1.0f;
                	m_gameWorldGrid[index].b = 1.0f;
                }
            }

            // Fix the corners where the opening is not towards the room
            if ((abs(m_gameWorldGrid[index].nx) == 1.0f) && (abs(m_gameWorldGrid[index].nz) == 1.0f))
            {
                m_gameWorldGrid[index].nx *= -1.0f;
                m_gameWorldGrid[index].nz *= -1.0f;
                m_gameWorldGrid[index].r = 1.0f;
            }
            
            // Now check for tiles that are corners
            if (m_gameWorldGrid[index].nx == 0.0f && m_gameWorldGrid[index].nz == 0.0f)
            {
            	if (northEastTile == ' ')
	            {
	            	m_gameWorldGrid[index].nx = 1.0f;
	            	m_gameWorldGrid[index].nz = 1.0f;
	            }
	            else if (northWestTile == ' ')
	            {
                    m_gameWorldGrid[index].nx = -1.0f;
	            	m_gameWorldGrid[index].nz = 1.0f;
	            }
	            else if (southEastTile == ' ')
	            {
	            	m_gameWorldGrid[index].nx = 1.0f;
	            	m_gameWorldGrid[index].nz = -1.0f;
	            }
	            else if (southWestTile == ' ')
	            {
	            	m_gameWorldGrid[index].nx = -1.0f;
	            	m_gameWorldGrid[index].nz = -1.0f;
	            }
            }
*/

/*            
            // Now check for tile that have corners
            if (northEastTile == ' ' || northWestTile == ' ')
            {
            	if (southEastTile == ' ' || southWestTile == ' ')
            	{
            		m_gameWorldGrid[index].nz = 2.0f;
            	}
            	else
            	{
            		m_gameWorldGrid[index].nz = 1.0f;
            	}
            }
            else if (southEastTile == ' ' || southWestTile == ' ')
            {
                m_gameWorldGrid[index].nz = -1.0f;
            }
*/
            // Set the y-direction normal to 0
            m_gameWorldGrid[index].ny = 0.0f;
          }
          else if (tile == ClosedDoor)
          {
          	m_gameWorldGrid[index].geoFeatures |= Doorway;
          	m_gameWorldGrid[index].ny = 1.0f;
          	/*
          	char northTile = GetTile(x, y+1);
          	char southTile = GetTile(x, y-1);
          	char eastTile  = GetTile(x+1, y);
          	char westTile  = GetTile(x-1, y);
          	
            if (northTile == Wall && southTile == Wall)
            {
                m_gameWorldGrid[index].nx = 0.0f;
                m_gameWorldGrid[index].ny = 1.0f;
                m_gameWorldGrid[index].nz = 2.0f;
            }
            else
            {
                m_gameWorldGrid[index].nx = 2.0f;
                m_gameWorldGrid[index].ny = 1.0f;
                m_gameWorldGrid[index].nz = 0.0f;
            }
            */
          }
          else
          {
          	m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
          	m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
          	m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
          	m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
           	m_gameWorldGrid[index].ny = 1.0f;
          }

          index++;
		}
	  }
	}
	
/*
	for(j=0; j<m_worldLength; j++)
	{
		for(i=0; i<m_worldWidth; i++)
		{
			index = (m_worldWidth * j) + i;
            
            // Set the position of the grid vertex
			m_gameWorldGrid[index].x = (float)i;
			m_gameWorldGrid[index].y = 0.0f;
			m_gameWorldGrid[index].z = (float)j;
            
            // Set the normal vector for that vertex
			m_gameWorldGrid[index].nx = 0.0f;
		    m_gameWorldGrid[index].ny = 1.0f;
		    m_gameWorldGrid[index].nz = 0.0f;
        }
	}
*/
	

	return true;
}


void GameWorld::ReleaseWorldGrid()
{
	if(m_gameWorldGrid)
	{
		delete [] m_gameWorldGrid;
		m_gameWorldGrid = nullptr;
	}
/*
	if(m_tiles)
	{
		delete m_tiles;
		m_tiles = nullptr;
	}

	if (m_rooms)
	{
		delete m_rooms;
		m_rooms = nullptr;
	}

	if (m_exits)
	{
		delete m_exits;
		m_exits = nullptr;
	}
*/
	return;
}


void GameWorld::CalculateTextureCoordinates()
{
	int incrementCount, tuCount, tvCount;
	uint32 i, j;
	float incrementValue, tuCoordinate, tvCoordinate;


	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)m_worldWidth;

	// Calculate how many times to repeat the texture.
	incrementCount = m_worldWidth / TEXTURE_REPEAT;

	// Initialize the tu and tv coordinate values.
	tuCoordinate = 0.0f;
	tvCoordinate = 1.0f;

	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;

	// Loop through the entire height map and calculate the tu and tv texture coordinates for each vertex.
	for(j=0; j<m_worldLength; j++)
	{
		for(i=0; i<m_worldWidth; i++)
		{
			// Store the texture coordinate in the height map.
			m_gameWorldGrid[(m_worldLength * j) + i].tu = tuCoordinate;
			m_gameWorldGrid[(m_worldLength * j) + i].tv = tvCoordinate;

			// Increment the tu texture coordinate by the increment value and increment the index by one.
			tuCoordinate += incrementValue;
			tuCount++;

			// Check if at the far right end of the texture and if so then start at the beginning again.
			if(tuCount == incrementCount)
			{
				tuCoordinate = 0.0f;
				tuCount = 0;
			}
		}

		// Increment the tv texture coordinate by the increment value and increment the index by one.
		tvCoordinate -= incrementValue;
		tvCount++;

		// Check if at the top of the texture and if so then start at the bottom again.
		if(tvCount == incrementCount)
		{
			tvCoordinate = 1.0f;
			tvCount = 0;
		}
	}

	return;
}


bool GameWorld::LoadTextures(ID3D11Device* device, LPCSTR* groundTextureFilename, LPCSTR* wallTextureFilename)
{
	bool result;


	// Create the ground texture object.
	m_GroundTexture = new Gumshoe::Texture;
	if(!m_GroundTexture)
	{
		return false;
	}

	// Initialize the ground texture object.
	result = m_GroundTexture->Init(device, groundTextureFilename);
	if(!result)
	{
		return false;
	}

	// Create the wall texture object.
	m_WallTexture = new Gumshoe::Texture;
	if(!m_WallTexture)
	{
		return false;
	}

	// Initialize the wall texture object.
	result = m_WallTexture->Init(device, wallTextureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


void GameWorld::ReleaseTextures()
{
	// Release the texture objects.
	if(m_GroundTexture)
	{
		m_GroundTexture->Shutdown();
		delete m_GroundTexture;
		m_GroundTexture = nullptr;
	}

	if(m_WallTexture)
	{
		m_WallTexture->Shutdown();
		delete m_WallTexture;
		m_WallTexture = nullptr;
	}

	return;
}


bool GameWorld::LoadColorMap(char* filename)
{
	int error, imageSize, index;
	uint32 i, j, k, colorMapLength, colorMapWidth;
	FILE* filePtr;
	size_t count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char* bitmapImage;


	// Open the color map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Make sure the color map dimensions are the same as the world dimensions for easy 1 to 1 mapping.
	colorMapLength = bitmapInfoHeader.biWidth;
	colorMapWidth = bitmapInfoHeader.biHeight;

	if((colorMapLength != m_worldLength) || (colorMapWidth != m_worldWidth))
	{
		return false;
	}

	// Calculate the size of the bitmap image data.
	imageSize = colorMapLength * colorMapWidth * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k=0;

	// Read the image data into the color map portion of the height map structure.
	for(j=0; j<colorMapLength; j++)
	{
		for(i=0; i<colorMapWidth; i++)
		{
			index = (colorMapLength * j) + i;

			m_gameWorldGrid[index].b = (float)bitmapImage[k]   / 255.0f;
			m_gameWorldGrid[index].g = (float)bitmapImage[k+1] / 255.0f;
			m_gameWorldGrid[index].r = (float)bitmapImage[k+2] / 255.0f;

			k+=3;
		}
	}

	// Release the bitmap image data.
	delete [] bitmapImage;
	bitmapImage = nullptr;

	return true;
}

/*
bool GameWorld::LoadMaterialFile(char* filename, char* materialMapFilename, ID3D11Device* device)
{
	ifstream fin;
	char input;
	uint32 i;
	//int error;
	char inputFilename[128];
	//unsigned int stringLength;
	LPCSTR textureFilename;
	bool result;


	// Open the materials information text file.
	fin.open(filename);
	if(fin.fail())
	{
		return false;
	}

	// Read up to the value of texture count.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}

	// Read in the texture count.
	fin >> m_textureCount;

	// Create the texture object array.
	m_Textures = new Texture[m_textureCount];
	if(!m_Textures)
	{
		return false;
	}

	// Load each of the textures in.
	for(i=0; i<m_textureCount; i++)
	{
		fin.get(input);
		while(input != ':')
		{
			fin.get(input);
		}

		fin >> inputFilename;

		// Convert the character filename to LPCSTR.
		textureFilename = (LPCSTR)inputFilename;
		
		//error = mbstowcs_s(&stringLength, textureFilename, 128, inputFilename, 128);
		//if(error != 0)
		//{
		//	return false;
		//}

		// Load the texture or alpha map.
		result = m_Textures[i].Init(device, &textureFilename);
		if(!result)
		{
			return false;
		}
	}

	// Read up to the value of the material count.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}

	// Read in the material count.
	fin >> m_materialCount;

	// Create the material group array.
	m_Materials = new materialGroup_t[m_materialCount];
	if(!m_Materials)
	{
		return false;
	}

	// Initialize the material group array.
	for(i=0; i<m_materialCount; i++)
	{
		m_Materials[i].vertexBuffer = nullptr;
		m_Materials[i].indexBuffer = nullptr;
		m_Materials[i].vertices = nullptr;
		m_Materials[i].indices = nullptr;
	}

	// Load each of the material group indexes in.
	for(i=0; i<m_materialCount; i++)
	{
		fin.get(input);
		while(input != ':')
		{
			fin.get(input);
		}

		fin >> m_Materials[i].textureIndex1 >> m_Materials[i].textureIndex2 >> m_Materials[i].alphaIndex;
		fin >> m_Materials[i].r >> m_Materials[i].g >> m_Materials[i].b;
	}

	// Close the materials information text file.
	fin.close();
	
	// Now load the material index map.
	result = LoadMaterialMap(materialMapFilename);
	if(!result)
	{
		return false;
	}

	// Load the vertex buffer for each material group with the world data.
	result = LoadMaterialBuffers(device);
	if(!result)
	{
		return false;
	}

	return true;
}


bool GameWorld::LoadMaterialMap(char* filename)
{
	uint32 i, j, k;
	uint32 matIndexMapLength, matIndexMapWidth;
	int error, imageSize, index;
	FILE* filePtr;
	size_t count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char* bitmapImage;


	// Open the material map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Make sure the material index map dimensions are the same as the world dimensions for 1 to 1 mapping.
	matIndexMapLength = bitmapInfoHeader.biWidth;
	matIndexMapWidth = bitmapInfoHeader.biHeight;

	if((matIndexMapLength != m_worldLength) || (matIndexMapWidth != m_worldWidth))
	{
		return false;
	}

	// Calculate the size of the bitmap image data.
	imageSize = bitmapInfoHeader.biWidth * bitmapInfoHeader.biHeight * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Initialize the position in the image data buffer so each vertice has an material index associated with it.
	k=0;

	// Read the material index data into the game world structure.
	for(j=0; j<m_worldWidth; j++)
	{
		for(i=0; i<m_worldLength; i++)
		{
			index = (m_worldWidth * j) + i;

			m_gameWorldGrid[index].mat_r = (int)bitmapImage[k+2];
			m_gameWorldGrid[index].mat_g = (int)bitmapImage[k+1];
			m_gameWorldGrid[index].mat_b = (int)bitmapImage[k];

			k+=3;
		}
	}

	// Release the bitmap image data.
	delete [] bitmapImage;
	bitmapImage = nullptr;

	return true;
}


bool GameWorld::LoadMaterialBuffers(ID3D11Device* device)
{
	uint32 i, j;
	int maxVertexCount, maxIndexCount, index1, index2, index3, index4, redIndex, greenIndex, blueIndex, index, vIndex;
	bool found;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;


	// Create the value for the maximum number of vertices a material group could possibly have.
	maxVertexCount = (m_worldLength - 1) * (m_worldWidth - 1) * 6;

	// Set the index count to the same as the maximum vertex count.
	maxIndexCount = maxVertexCount;

	// Initialize vertex and index arrays for each material group to the maximum size.
	for(i=0; i<m_materialCount; i++)
	{
		// Create the temporary vertex array for this material group.
		m_Materials[i].vertices = new gameWorldVertex_t[maxVertexCount];
		if(!m_Materials[i].vertices)
		{
			return false;
		}

		// Create the temporary index array for this material group.
		m_Materials[i].indices = new unsigned long[maxIndexCount];
		if(!m_Materials[i].indices)
		{
			return false;
		}

		// Initialize the counts to zero.
		m_Materials[i].vertexCount = 0;
		m_Materials[i].indexCount = 0;
	}

	// Now loop through the world and build the vertex arrays for each material group.
	for(j=0; j<(m_worldWidth-1); j++)
	{
		for(i=0; i<(m_worldLength-1); i++)
		{
			index1 = (m_worldWidth * j) + i;          // Bottom left.
			index2 = (m_worldWidth * j) + (i+1);      // Bottom right.
			index3 = (m_worldWidth * (j+1)) + i;      // Upper left.
			index4 = (m_worldWidth * (j+1)) + (i+1);  // Upper right.

			// Query the upper left corner vertex for the material index.
			redIndex   = m_gameWorldGrid[index3].mat_r;
			greenIndex = m_gameWorldGrid[index3].mat_g;
			blueIndex  = m_gameWorldGrid[index3].mat_b;

			// Find which material group this vertex belongs to.
			index = 0;
			found = false;
			while(!found)
			{
				if((redIndex == m_Materials[index].r) && (greenIndex == m_Materials[index].g) && (blueIndex == m_Materials[index].b))
				{
					found = true;
				}
				else
				{
					index++;
				}
			}

			// Set the index position in the vertex and index array to the count.
			vIndex = m_Materials[index].vertexCount;

			// Upper left.
			m_Materials[index].vertices[vIndex].position = D3DXVECTOR3(m_gameWorldGrid[index3].x, m_gameWorldGrid[index3].y, m_gameWorldGrid[index3].z);
			m_Materials[index].vertices[vIndex].texture = D3DXVECTOR2(0.0f, 0.0f);
			m_Materials[index].vertices[vIndex].normal = D3DXVECTOR3(m_gameWorldGrid[index3].nx, m_gameWorldGrid[index3].ny, m_gameWorldGrid[index3].nz);
			m_Materials[index].vertices[vIndex].color = D3DXVECTOR4(m_gameWorldGrid[index3].r, m_gameWorldGrid[index3].g, m_gameWorldGrid[index3].b, 1.0f);
			m_Materials[index].indices[vIndex] = vIndex;
			vIndex++;

			// Upper right.
			m_Materials[index].vertices[vIndex].position = D3DXVECTOR3(m_gameWorldGrid[index4].x, m_gameWorldGrid[index4].y, m_gameWorldGrid[index4].z);
			m_Materials[index].vertices[vIndex].texture = D3DXVECTOR2(1.0f, 0.0f);
			m_Materials[index].vertices[vIndex].normal = D3DXVECTOR3(m_gameWorldGrid[index4].nx, m_gameWorldGrid[index4].ny, m_gameWorldGrid[index4].nz);
			m_Materials[index].vertices[vIndex].color = D3DXVECTOR4(m_gameWorldGrid[index4].r, m_gameWorldGrid[index4].g, m_gameWorldGrid[index4].b, 1.0f);
			m_Materials[index].indices[vIndex] = vIndex;
			vIndex++;

			// Bottom left.
			m_Materials[index].vertices[vIndex].position = D3DXVECTOR3(m_gameWorldGrid[index1].x, m_gameWorldGrid[index1].y, m_gameWorldGrid[index1].z);
			m_Materials[index].vertices[vIndex].texture = D3DXVECTOR2(0.0f, 1.0f);
			m_Materials[index].vertices[vIndex].normal = D3DXVECTOR3(m_gameWorldGrid[index1].nx, m_gameWorldGrid[index1].ny, m_gameWorldGrid[index1].nz);
			m_Materials[index].vertices[vIndex].color = D3DXVECTOR4(m_gameWorldGrid[index1].r, m_gameWorldGrid[index1].g, m_gameWorldGrid[index1].b, 1.0f);
			m_Materials[index].indices[vIndex] = vIndex;
			vIndex++;

			// Bottom left.
			m_Materials[index].vertices[vIndex].position = D3DXVECTOR3(m_gameWorldGrid[index1].x, m_gameWorldGrid[index1].y, m_gameWorldGrid[index1].z);
			m_Materials[index].vertices[vIndex].texture = D3DXVECTOR2(0.0f, 1.0f);
			m_Materials[index].vertices[vIndex].normal = D3DXVECTOR3(m_gameWorldGrid[index1].nx, m_gameWorldGrid[index1].ny, m_gameWorldGrid[index1].nz);
			m_Materials[index].vertices[vIndex].color = D3DXVECTOR4(m_gameWorldGrid[index1].r, m_gameWorldGrid[index1].g, m_gameWorldGrid[index1].b, 1.0f);
			m_Materials[index].indices[vIndex] = vIndex;
			vIndex++;

			// Upper right.
			m_Materials[index].vertices[vIndex].position = D3DXVECTOR3(m_gameWorldGrid[index4].x, m_gameWorldGrid[index4].y, m_gameWorldGrid[index4].z);
			m_Materials[index].vertices[vIndex].texture = D3DXVECTOR2(1.0f, 0.0f);
			m_Materials[index].vertices[vIndex].normal = D3DXVECTOR3(m_gameWorldGrid[index4].nx, m_gameWorldGrid[index4].ny, m_gameWorldGrid[index4].nz);
			m_Materials[index].vertices[vIndex].color = D3DXVECTOR4(m_gameWorldGrid[index4].r, m_gameWorldGrid[index4].g, m_gameWorldGrid[index4].b, 1.0f);
			m_Materials[index].indices[vIndex] = vIndex;
			vIndex++;

			// Bottom right.
			m_Materials[index].vertices[vIndex].position = D3DXVECTOR3(m_gameWorldGrid[index2].x, m_gameWorldGrid[index2].y, m_gameWorldGrid[index2].z);
			m_Materials[index].vertices[vIndex].texture = D3DXVECTOR2(1.0f, 1.0f);
			m_Materials[index].vertices[vIndex].normal = D3DXVECTOR3(m_gameWorldGrid[index2].nx, m_gameWorldGrid[index2].ny, m_gameWorldGrid[index2].nz);
			m_Materials[index].vertices[vIndex].color = D3DXVECTOR4(m_gameWorldGrid[index2].r, m_gameWorldGrid[index2].g, m_gameWorldGrid[index2].b, 1.0f);
			m_Materials[index].indices[vIndex] = vIndex;
			vIndex++;

			// Increment the vertex and index array counts.
			m_Materials[index].vertexCount += 6;
			m_Materials[index].indexCount += 6;
		}
	}

	// Now create the vertex and index buffers from the vertex and index arrays for each material group.
	for(i=0; i<m_materialCount; i++)
	{
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(gameWorldVertex_t) * m_Materials[i].vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		vertexData.pSysMem = m_Materials[i].vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

	    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_Materials[i].vertexBuffer);
		if(FAILED(result))
		{
			return false;
		}

	    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_Materials[i].indexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

	    indexData.pSysMem = m_Materials[i].indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_Materials[i].indexBuffer);
		if(FAILED(result))
		{
			return false;
		}

		//delete [] m_Materials[i].vertices;
		//m_Materials[i].vertices = nullptr;

		delete [] m_Materials[i].indices;
		m_Materials[i].indices = nullptr;
	}

	return true;
}


void GameWorld::ReleaseMaterials()
{
	uint32 i;


	// Release the material groups.
	if(m_Materials)
	{
		for(i=0; i<m_materialCount; i++)
		{
			if(m_Materials[i].vertexBuffer)
			{
				m_Materials[i].vertexBuffer->Release();
				m_Materials[i].vertexBuffer = nullptr;
			}

			if(m_Materials[i].indexBuffer)
			{
				m_Materials[i].indexBuffer->Release();
				m_Materials[i].indexBuffer = nullptr;
			}

			if(m_Materials[i].vertices)
			{
				delete [] m_Materials[i].vertices;
				m_Materials[i].vertices = nullptr;
			}

			if(m_Materials[i].indices)
			{
				delete [] m_Materials[i].indices;
				m_Materials[i].indices = nullptr;
			}
		}

		delete [] m_Materials;
		m_Materials = nullptr;
	}

	// Release the terrain textures and alpha maps.
	if(m_Textures)
	{
		for(i=0; i<m_textureCount; i++)
		{
			m_Textures[i].Shutdown();
		}

		delete [] m_Textures;
		m_Textures = nullptr;
	}

	return;
}


bool GameWorld::Render(ID3D11DeviceContext* deviceContext, Gumshoe::Shader* shader, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
					   D3DXMATRIX projectionMatrix, D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection)
{
	unsigned int stride;
	unsigned int offset;
	uint32 i;
	bool result;


	// Set vertex buffer stride and offset.
	stride = sizeof(gameWorldVertex_t); 
	offset = 0;

	// Set the shader parameters that it will use for rendering.
	result = shader->SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, ambientColor, diffuseColor, lightDirection);
	
	// Set the game map shader parameters that it will use for rendering.
	//result = m_Shader->PublicSetShaderParameters(m_Direct3DSystem->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
	//				                             m_World->GetTexture(), m_Light->GetDirection(), m_Light->GetAmbientColor(), m_Light->GetDiffuseColor(),
	//				                             m_Camera->GetPosition(), {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 3);
	if(!result)
	{
		return false;
	}

	// Set the type of primitive that should be rendered from the vertex buffers, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render each material group.
	for(i=0; i<m_materialCount; i++)
	{
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetVertexBuffers(0, 1, &m_Materials[i].vertexBuffer, &stride, &offset);

	    // Set the index buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetIndexBuffer(m_Materials[i].indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// If the material group has a valid second texture index then this is a blended terrain polygon.
		if(m_Materials[i].textureIndex2 != -1)
		{
			result = shader->SetShaderTextures(deviceContext,  m_Textures[m_Materials[i].textureIndex1].GetTexture(),  
											   m_Textures[m_Materials[i].textureIndex2].GetTexture(),  m_Textures[m_Materials[i].alphaIndex].GetTexture(), 
											   true);
		}
        // If not then it is just a single textured polygon.
		else
		{
			result = shader->SetShaderTextures(deviceContext, m_Textures[m_Materials[i].textureIndex1].GetTexture(), NULL, NULL, false);
		}

		// Check if the textures were set or not.
		if(!result)
		{
			return false;
		}

		// Now render the prepared buffers with the shader.
		shader->PublicRenderShader(deviceContext, m_Materials[i].indexCount);
	}

	return true;
}
*/

void GameWorld::CreateWallGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                               std::vector<unsigned long> &indices, uint32 &index)
{
    gameWorldVertex_t currVertex;
    int j;

    float xFloorOffset = 0.6f;
    float zFloorOffset = 0.0f;
    float xFloorWidth = 0.4f;
    float zFloorWidth = 1.0f;

    float xWallOffset = 0.6f;
    float zWallOffset = 0.0f;
    float xWallWidth = -0.2f;
    float zWallWidth = 1.0f;
    
    float xWallPos1 = 0.0f;
    float xWallPos2 = -0.2f;
    float xWallPos3 = -0.2f;
    float zWallPos1 = 1.0f;
    float zWallPos2 = 1.0f;
    float zWallPos3 = 0.0f;
   
    float xNormOffset = -2.0f;
    float zNormOffset = 0.0f;

    // Check the normals to augment the wall creation (East facing is the default)
    // West facing wall
    if (m_gameWorldGrid[tileIndex].nx == -1.0f)
    {
        xFloorOffset = 0.0f;

        xWallOffset = 0.4f;
        zWallOffset = 1.0f;
        xWallWidth = 0.2f;
        zWallWidth = -1.0f;

        xWallPos2 = 0.2f;
        xWallPos3 = 0.2f;
        zWallPos1 = -1.0f;
        zWallPos2 = -1.0f;
        
        xNormOffset = 2.0f;
    }

    // North facing wall
    if (m_gameWorldGrid[tileIndex].nz == 1.0f)
    {  
    	xFloorOffset = 0.0f;
        zFloorOffset = 0.6f;
        xFloorWidth = 1.0f;
        zFloorWidth = 0.4f;

        xWallOffset = 0.0f;
        zWallOffset = 0.6f;
        xWallWidth = 1.0f;
        zWallWidth = -0.2f;

        xWallPos1 = 1.0f;
	    xWallPos2 = 0.0f;
	    xWallPos3 = 1.0f;
	    zWallPos1 = 0.0f;
	    zWallPos2 = -0.2f;
	    zWallPos3 = -0.2f;
        
        xNormOffset = 0.0f;
        zNormOffset = -2.0f;
    }

    // South facing wall
    if (m_gameWorldGrid[tileIndex].nz == -1.0f)
    {
        xFloorOffset = 0.0f;
        zFloorOffset = 0.0f;
        xFloorWidth = 1.0f;
        zFloorWidth = 0.4f;

        xWallOffset = 1.0f;
        zWallOffset = 0.4f;
        xWallWidth = -1.0f;
        zWallWidth = 0.2f;

        xWallPos1 = -1.0f;
	    xWallPos2 = 0.0f;
	    xWallPos3 = -1.0f;
	    zWallPos1 = 0.0f;
	    zWallPos2 = 0.2f;
	    zWallPos3 = 0.2f;
        
        xNormOffset = 0.0f;
        zNormOffset = 2.0f;
    }

    // Both East and West facing wall
    if (m_gameWorldGrid[tileIndex].nx == 2.0f)
    {
        // Treat this as an east facing wall, but first add in the west facing partial floor
        xFloorOffset = 0.0f;
        zFloorOffset = 0.0f;
        xFloorWidth = 0.4f;
        zFloorWidth = 1.0f;

        xWallOffset = 0.6f;
        zWallOffset = 0.0f;
        xWallWidth = -0.2f;
        zWallWidth = 1.0f;
    
        xWallPos1 = 0.0f;
        xWallPos2 = -0.2f;
        xWallPos3 = -0.2f;
        zWallPos1 = 1.0f;
        zWallPos2 = 1.0f;
        zWallPos3 = 0.0f;
   
        xNormOffset = -2.0f;
        zNormOffset = 0.0f;

        // Check if this is an end wall, and if so, add in the edge wall part
        if (m_gameWorldGrid[tileIndex].nz != 0.0f)
        {
            // Wall is 3m high
		    for (j = 0; j < 3; j++)
		    {
		        float zPos = m_gameWorldGrid[tileIndex].z + 0.5f + (m_gameWorldGrid[tileIndex].nz*0.5f);
		        float yPos = m_gameWorldGrid[tileIndex].y + ((float)j*1.0f);
		        float xPos = m_gameWorldGrid[tileIndex].x + 0.5f + (m_gameWorldGrid[tileIndex].nz*0.1f);
		        float xPos2 = xPos + (-0.2f*m_gameWorldGrid[tileIndex].nz);

		        // Bottom left corner of tile (vertex 0)
		        currVertex.position = D3DXVECTOR3(xPos, yPos, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 0.4f);
			    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, m_gameWorldGrid[tileIndex].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top left corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(xPos, yPos + 1.0f, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 0.4f);
			    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, m_gameWorldGrid[tileIndex].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top right corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(xPos2, yPos + 1.0f, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 0.6f);
			    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, m_gameWorldGrid[tileIndex].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

                // Top right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(xPos2, yPos + 1.0f, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 0.6f);
			    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, m_gameWorldGrid[tileIndex].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(xPos2, yPos, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 0.6f);
			    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, m_gameWorldGrid[tileIndex].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom left corner of tile (vertex 1)
		        currVertex.position = D3DXVECTOR3(xPos, yPos, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 0.4f);
			    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, m_gameWorldGrid[tileIndex].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;
			}
        }

        // Bottom left corner of tile (vertex 0)
	    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - zFloorOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 1)
	    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		xFloorOffset = 0.6f;
		m_gameWorldGrid[tileIndex].nx = 1.0f;
		m_gameWorldGrid[tileIndex].nz = 0.0f;
    }

    // Check if this is a North and South facing wall, if so, add the south facing floor partial
    if (m_gameWorldGrid[tileIndex].nz == 2.0f)
    {
    	xFloorOffset = 0.0f;
        zFloorOffset = 0.0f;
        xFloorWidth = 1.0f;
        zFloorWidth = 0.4f;

        // Check if this is an end wall, and if so, add in the edge wall part
        if (m_gameWorldGrid[tileIndex].nx != 0.0f)
        {
            // Wall is 3m high
		    for (j = 0; j < 3; j++)
		    {
		        float xPos = m_gameWorldGrid[tileIndex].x + 0.5f + (m_gameWorldGrid[tileIndex].nx*0.5f);
		        float yPos = m_gameWorldGrid[tileIndex].y + ((float)j*1.0f);
		        float zPos = m_gameWorldGrid[tileIndex].z + 0.5f + (m_gameWorldGrid[tileIndex].nx*-0.1f);
		        float zPos2 = zPos + (0.2f*m_gameWorldGrid[tileIndex].nx);

		        // Bottom left corner of tile (vertex 0)
		        currVertex.position = D3DXVECTOR3(xPos, yPos, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 0.4f);
			    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, 0.0f, 0.0f);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top left corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(xPos, yPos + 1.0f, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 0.4f);
			    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, 0.0f, 0.0f);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top right corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(xPos, yPos + 1.0f, zPos2);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 0.6f);
			    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, 0.0f, 0.0f);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

                // Top right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(xPos, yPos + 1.0f, zPos2);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 0.6f);
			    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, 0.0f, 0.0f);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(xPos, yPos, zPos2);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 0.6f);
			    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, 0.0f, 0.0f);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom left corner of tile (vertex 1)
		        currVertex.position = D3DXVECTOR3(xPos, yPos, zPos);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 0.4f);
			    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, 0.0f, 0.0f);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;
			}
		}

        // Bottom left corner of tile (vertex 0)
	    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - zFloorOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 1)
	    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;
    
        xFloorOffset = 0.0f;
        zFloorOffset = 0.6f;
        xFloorWidth = 1.0f;
        zFloorWidth = 0.4f;

        xWallOffset = 0.0f;
        zWallOffset = 0.6f;
        xWallWidth = 1.0f;
        zWallWidth = -0.2f;

        xWallPos1 = 1.0f;
	    xWallPos2 = 0.0f;
	    xWallPos3 = 1.0f;
	    zWallPos1 = 0.0f;
	    zWallPos2 = -0.2f;
	    zWallPos3 = -0.2f;
        
        xNormOffset = 0.0f;
        zNormOffset = -2.0f;

        m_gameWorldGrid[tileIndex].nx = 0.0f;
		m_gameWorldGrid[tileIndex].nz = 1.0f;
    }


	// Add in the partial floor area
    // Bottom left corner of tile (vertex 0)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top left corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top right corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Top right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom left corner of tile (vertex 1)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Add in the top of the wall
    // Bottom left corner of tile (vertex 0)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].tv - zWallOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top left corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].tv - (zWallOffset + zWallWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top right corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWallOffset, m_gameWorldGrid[tileIndex].tv - (zWallOffset + zWallWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Top right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWallOffset, m_gameWorldGrid[tileIndex].tv - (zWallOffset + zWallWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWallOffset, m_gameWorldGrid[tileIndex].tv - zWallOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom left corner of tile (vertex 1)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].tv - zWallOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

    // Wall is 3m high
    for (j = 0; j < 3; j++)
    {
        float yPos = m_gameWorldGrid[tileIndex].y + ((float)j*1.0f);

        // Bottom left corner of tile (vertex 0)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos1), yPos, m_gameWorldGrid[tileIndex].z + zWallOffset);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos1), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos1));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos1));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos1));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 1)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos1), yPos, m_gameWorldGrid[tileIndex].z + zWallOffset);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

        
        // Other side of the wall
		// Bottom left corner of tile (vertex 2)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos2), yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos2));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx + xNormOffset, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz + zNormOffset);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 2)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos2), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos2));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx + xNormOffset, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz + zNormOffset);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 2)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos3), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos3));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx + xNormOffset, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz + zNormOffset);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 3)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos3), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos3));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx + xNormOffset, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz + zNormOffset);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 3)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos3), yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos3));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx + xNormOffset, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz + zNormOffset);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 3)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos2), yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos2));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[tileIndex].nx + xNormOffset, m_gameWorldGrid[tileIndex].ny, m_gameWorldGrid[tileIndex].nz + zNormOffset);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;
    }
}


void GameWorld::CreateWallCornerGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                                     std::vector<unsigned long> &indices, uint32 &index)
{
    gameWorldVertex_t currVertex;
    int j;

    float xFloorOffset = 0.6f;
    float zFloorOffset = 0.6f;
    float xFloorWidth = 0.4f;
    float zFloorWidth = 0.4f;

    float xWall1Offset = 0.6f;
    float zWall1Offset = 0.4f;
    float xWall1Width = -0.2f;
    float zWall1Width = 0.6f;

    float xWall2Offset = 0.4f;
    float zWall2Offset = 0.4f;
    float xWall2Width = 0.6f;
    float zWall2Width = 0.2f;
    
    float xNormOffset = -2.0f;
    float zNormOffset = -2.0f;

    // Check the r-value for the tile to see if it is an outward facing corner. If so, we have to make two floor partials.
    if (m_gameWorldGrid[tileIndex].r == 1.0f)
    {
        xFloorOffset = 0.0f;
        zFloorOffset = 0.4f;
        xFloorWidth = 0.4f;
        zFloorWidth = 0.6f;
    }

    // Check the normals to augment the corner creation (North-East facing is the default)
    // North-West facing corner
    if (m_gameWorldGrid[tileIndex].nx == -1.0f && m_gameWorldGrid[tileIndex].nz == 1.0f)
    {
        if (m_gameWorldGrid[tileIndex].r == 1.0f)
        {
            xFloorOffset = 0.6f;
	        zFloorOffset = 0.4f;
	        xFloorWidth = 0.4f;
	        zFloorWidth = 0.6f;
        }
        else
        {
	        xFloorOffset = 0.0f;
	        zFloorOffset = 0.6f;
	        xFloorWidth = 0.4f;
	        zFloorWidth = 0.4f;
        }

        xWall1Offset = 0.6f;
        zWall1Offset = 0.4f;
        xWall1Width = -0.2f;
        zWall1Width = 0.6f;

        xWall2Offset = 0.0f;
        zWall2Offset = 0.4f;
        xWall2Width = 0.6f;
        zWall2Width = 0.2f;

        xNormOffset = 2.0f;
        zNormOffset = -2.0f;
    }
    // South-East facing corner
    else if (m_gameWorldGrid[tileIndex].nx == 1.0f && m_gameWorldGrid[tileIndex].nz == -1.0f)
    {
        if (m_gameWorldGrid[tileIndex].r == 1.0f)
        {
            xFloorOffset = 0.0f;
	        zFloorOffset = 0.0f;
	        xFloorWidth = 0.4f;
	        zFloorWidth = 0.6f;
        }
        else
        {
	        xFloorOffset = 0.6f;
	        zFloorOffset = 0.0f;
	        xFloorWidth = 0.4f;
	        zFloorWidth = 0.4f;
        }

        xWall1Offset = 0.6f;
        zWall1Offset = 0.0f;
        xWall1Width = -0.2f;
        zWall1Width = 0.6f;

        xWall2Offset = 0.4f;
        zWall2Offset = 0.4f;
        xWall2Width = 0.6f;
        zWall2Width = 0.2f;

        xNormOffset = -2.0f;
        zNormOffset = 2.0f;
    }
    // South-West facing corner
    else if (m_gameWorldGrid[tileIndex].nx == -1.0f && m_gameWorldGrid[tileIndex].nz == -1.0f)
    {
        if (m_gameWorldGrid[tileIndex].r == 1.0f)
        {
            xFloorOffset = 0.6f;
	        zFloorOffset = 0.0f;
	        xFloorWidth = 0.4f;
	        zFloorWidth = 0.6f;
        }
        else
        {
	        xFloorOffset = 0.0f;
	        zFloorOffset = 0.0f;
	        xFloorWidth = 0.4f;
	        zFloorWidth = 0.4f;
        }

        xWall1Offset = 0.6f;
        zWall1Offset = 0.0f;
        xWall1Width = -0.2f;
        zWall1Width = 0.6f;

        xWall2Offset = 0.0f;
        zWall2Offset = 0.4f;
        xWall2Width = 0.6f;
        zWall2Width = 0.2f;

        xNormOffset = 2.0f;
        zNormOffset = 2.0f;
    }

    // Add in the x-direction partial floor if it is an outward facing corner
    if (m_gameWorldGrid[tileIndex].r == 1.0f)
    {
    	float zOffset = zFloorOffset + zFloorWidth;
    	
    	if (zOffset == 1.0f)
    		zOffset = 0.0f;

    	// Bottom left corner of tile (vertex 0)
	    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - zOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zOffset + 0.4f));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - (zOffset + 0.4f));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + 1.0f, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zOffset + 0.4f));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - (zOffset + 0.4f));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + 1.0f, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zOffset + 0.4f));
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - (zOffset + 0.4f));
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + 1.0f, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - zOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 1)
	    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zOffset);
	    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - zOffset);
	    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
		indices.push_back(index);
		index++;
    }

    m_gameWorldGrid[tileIndex].r = 0.0f;

    // Add in the partial floor area
    // Bottom left corner of tile (vertex 0)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top left corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top right corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Top right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom left corner of tile (vertex 1)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Add in the top of the wall (part 1)
    // Bottom left corner of tile (vertex 0)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWall1Offset + xWall1Width), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWall1Offset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWall1Offset + xWall1Width), m_gameWorldGrid[tileIndex].tv - zWall1Offset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top left corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWall1Offset + xWall1Width), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWall1Offset + zWall1Width));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWall1Offset + xWall1Width), m_gameWorldGrid[tileIndex].tv - (zWall1Offset + zWall1Width));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top right corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWall1Offset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWall1Offset + zWall1Width));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWall1Offset, m_gameWorldGrid[tileIndex].tv - (zWall1Offset + zWall1Width));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Top right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWall1Offset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWall1Offset + zWall1Width));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWall1Offset, m_gameWorldGrid[tileIndex].tv - (zWall1Offset + zWall1Width));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWall1Offset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWall1Offset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWall1Offset, m_gameWorldGrid[tileIndex].tv - zWall1Offset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom left corner of tile (vertex 1)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWall1Offset + xWall1Width), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWall1Offset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWall1Offset + xWall1Width), m_gameWorldGrid[tileIndex].tv - zWall1Offset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Add in the top of the wall (part 2)
    // Bottom left corner of tile (vertex 0)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWall2Offset + xWall2Width), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWall2Offset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWall2Offset + xWall2Width), m_gameWorldGrid[tileIndex].tv - zWall2Offset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top left corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWall2Offset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWall2Offset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWall2Offset, m_gameWorldGrid[tileIndex].tv - zWall2Offset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top right corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWall2Offset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWall2Offset + zWall2Width));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWall2Offset, m_gameWorldGrid[tileIndex].tv - (zWall2Offset + zWall2Width));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Top right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWall2Offset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWall2Offset + zWall2Width));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWall2Offset, m_gameWorldGrid[tileIndex].tv - (zWall2Offset + zWall2Width));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWall2Offset + xWall2Width), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWall2Offset + zWall2Width));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWall2Offset + xWall2Width), m_gameWorldGrid[tileIndex].tv - (zWall2Offset + zWall2Width));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom left corner of tile (vertex 1)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWall2Offset + xWall2Width), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWall2Offset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWall2Offset + xWall2Width), m_gameWorldGrid[tileIndex].tv - zWall2Offset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Wall is 3m high
    for (j = 0; j < 6; j++)
    {
        float yPos = m_gameWorldGrid[tileIndex].y + ((float)(j%3)*1.0f);
        float side1Temp1;
        float side1Temp2;
        float side1Temp3;
        float side1Temp4;
        float side2Temp1;
        float side2Temp2;
        float side2Temp3;
        float side2Temp4;

        float xNormFront;
        float xNormBack;
        float zNormFront;
        float zNormBack;

        if (j < 3)
        {
            side1Temp1 = xWall1Offset;
            side1Temp2 = zWall1Offset;
            side1Temp3 = zWall1Offset + zWall1Width;
            side1Temp4 = xWall1Offset;

            side2Temp1 = xWall1Offset + xWall1Width;
            side2Temp2 = zWall1Offset;
            side2Temp3 = zWall1Offset + zWall1Width;
            side2Temp4 = xWall1Offset + xWall1Width;

            xNormFront = m_gameWorldGrid[tileIndex].nx;
            xNormBack = m_gameWorldGrid[tileIndex].nx + xNormOffset;
            zNormFront = 0.0f;
            zNormBack = 0.0f;
        }
        else
        {
            side1Temp1 = xWall2Offset + xWall2Width;
            side1Temp2 = zWall2Offset + zWall2Width;
            side1Temp3 = zWall2Offset + zWall2Width;
            side1Temp4 = xWall2Offset;

            side2Temp1 = xWall2Offset;
            side2Temp2 = zWall2Offset;
            side2Temp3 = zWall2Offset;
            side2Temp4 = xWall2Offset + xWall2Width;

            xNormFront = 0.0f;
            xNormBack = 0.0f;
            zNormFront = m_gameWorldGrid[tileIndex].nz;
            zNormBack = m_gameWorldGrid[tileIndex].nz + zNormOffset;
        }

        // Bottom left corner of tile (vertex 0)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side1Temp1, yPos, m_gameWorldGrid[tileIndex].z + side1Temp2);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side1Temp1, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + side1Temp2);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side1Temp4, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + side1Temp3);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side1Temp4, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + side1Temp3);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side1Temp4, yPos, m_gameWorldGrid[tileIndex].z + side1Temp3);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 1)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side1Temp1, yPos, m_gameWorldGrid[tileIndex].z + side1Temp2);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

        
        // Other side of the wall
		// Bottom left corner of tile (vertex 2)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side2Temp1, yPos, m_gameWorldGrid[tileIndex].z + side2Temp3);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 2)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side2Temp1, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + side2Temp3);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 2)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side2Temp4, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + side2Temp2);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 3)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side2Temp4, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + side2Temp2);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 3)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side2Temp4, yPos, m_gameWorldGrid[tileIndex].z + side2Temp2);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 3)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + side2Temp1, yPos, m_gameWorldGrid[tileIndex].z + side2Temp3);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;
    }
}


void GameWorld::CreateHalfWallGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                                   std::vector<unsigned long> &indices, uint32 &index)
{
    gameWorldVertex_t currVertex;
    int j;

    float xFloorOffset = 0.6f;
    float zFloorOffset = 0.6f;
    float xFloorWidth = 0.4f;
    float zFloorWidth = 0.4f;

    float xWallOffset = 0.6f;
    float zWallOffset = 0.6f;
    float xWallWidth = -0.2f;
    float zWallWidth = 0.4f;
    
    float xWallPos1 = 0.0f;
    float xWallPos2 = -0.2f;
    float xWallPos3 = -0.2f;
    float zWallPos1 = 0.4f;
    float zWallPos2 = 0.4f;
    float zWallPos3 = 0.0f;
   
    float xNormFront = 1.0f;
    float xNormBack = -1.0f;
    float zNormFront = 0.0f;
    float zNormBack = 0.0f;

    
    // Check the r,g,b values to see the direction and position of the half-wall
    // NorthEast segment half-wall
    if (m_gameWorldGrid[tileIndex].r == 1.0f && m_gameWorldGrid[tileIndex].b == 1.0f)
    {
        xFloorOffset = 0.6f;
		zFloorOffset = 0.6f;
		xFloorWidth = 0.4f;
		zFloorWidth = 0.4f;

        if (m_gameWorldGrid[tileIndex].g == 1.0f)
        {
            xWallOffset = 0.6f;
		    zWallOffset = 0.6f;
		    xWallWidth = -0.2f;
		    zWallWidth = 0.4f;
		    
		    xWallPos1 = 0.0f;
		    xWallPos2 = -0.2f;
		    xWallPos3 = -0.2f;
		    zWallPos1 = 0.4f;
		    zWallPos2 = 0.4f;
		    zWallPos3 = 0.0f;

            xNormFront = 1.0f;
            xNormBack  = -1.0f;
            zNormFront = 0.0f;
            zNormBack  = 0.0f;
        }
        else
        {
            xWallOffset = 0.6f;
		    zWallOffset = 0.6f;
		    xWallWidth = 0.4f;
		    zWallWidth = -0.2f;
		    
		    xWallPos1 = 0.4f;
		    xWallPos2 = -0.2f;
		    xWallPos3 = 0.4f;
		    zWallPos1 = 0.0f;
		    zWallPos2 = -0.2f;
		    zWallPos3 = -0.2f;

            xNormFront = 0.0f;
            xNormBack  = 0.0f;
            zNormFront = 1.0f;
            zNormBack  = -1.0f;
        }
    }
    // NorthWest segment half-wall
    else if (m_gameWorldGrid[tileIndex].r == -1.0f && m_gameWorldGrid[tileIndex].b == 1.0f)
    {
        xFloorOffset = 0.0f;
		zFloorOffset = 0.6f;
		xFloorWidth = 0.4f;
		zFloorWidth = 0.4f;

        if (m_gameWorldGrid[tileIndex].g == 1.0f)
        {
            xWallOffset = 0.6f;
		    zWallOffset = 0.6f;
		    xWallWidth = -0.2f;
		    zWallWidth = 0.4f;
		    
		    xWallPos1 = 0.0f;
		    xWallPos2 = -0.2f;
		    xWallPos3 = -0.2f;
		    zWallPos1 = 0.4f;
		    zWallPos2 = 0.4f;
		    zWallPos3 = 0.0f;

            xNormFront = 1.0f;
            xNormBack  = -1.0f;
            zNormFront = 0.0f;
            zNormBack  = 0.0f;
        }
        else
        {
            xWallOffset = 0.0f;
		    zWallOffset = 0.6f;
		    xWallWidth = 0.4f;
		    zWallWidth = -0.2f;
		    
		    xWallPos1 = 0.4f;
		    xWallPos2 = 0.0f;
		    xWallPos3 = 0.6f;
		    zWallPos1 = 0.0f;
		    zWallPos2 = -0.2f;
		    zWallPos3 = -0.2f;

            xNormFront = 0.0f;
            xNormBack  = 0.0f;
            zNormFront = 1.0f;
            zNormBack  = -1.0f;
        }
    }
    // SouthEast segment half-wall
    else if (m_gameWorldGrid[tileIndex].r == 1.0f && m_gameWorldGrid[tileIndex].b == -1.0f)
    {
        xFloorOffset = 0.6f;
		zFloorOffset = 0.0f;
		xFloorWidth = 0.4f;
		zFloorWidth = 0.4f;

        if (m_gameWorldGrid[tileIndex].g == 1.0f)
        {
            xWallOffset = 0.6f;
		    zWallOffset = 0.0f;
		    xWallWidth = -0.2f;
		    zWallWidth = 0.4f;
		    
		    xWallPos1 = 0.0f;
		    xWallPos2 = -0.2f;
		    xWallPos3 = -0.2f;
		    zWallPos1 = 0.4f;
		    zWallPos2 = 0.4f;
		    zWallPos3 = 0.0f;

            xNormFront = 1.0f;
            xNormBack  = -1.0f;
            zNormFront = 0.0f;
            zNormBack  = 0.0f;
        }
        else
        {
           xWallOffset = 0.6f;
		    zWallOffset = 0.6f;
		    xWallWidth = 0.4f;
		    zWallWidth = -0.2f;
		    
		    xWallPos1 = 0.4f;
		    xWallPos2 = -0.2f;
		    xWallPos3 = 0.4f;
		    zWallPos1 = 0.0f;
		    zWallPos2 = -0.2f;
		    zWallPos3 = -0.2f;

            xNormFront = 0.0f;
            xNormBack  = 0.0f;
            zNormFront = 1.0f;
            zNormBack  = -1.0f;
        }
    }
    // SouthWest segment half-wall
    else if (m_gameWorldGrid[tileIndex].r == -1.0f && m_gameWorldGrid[tileIndex].b == -1.0f)
    {
        xFloorOffset = 0.0f;
		zFloorOffset = 0.0f;
		xFloorWidth = 0.4f;
		zFloorWidth = 0.4f;

        if (m_gameWorldGrid[tileIndex].g == 1.0f)
        {
            xWallOffset = 0.6f;
		    zWallOffset = 0.0f;
		    xWallWidth = -0.2f;
		    zWallWidth = 0.4f;
		    
		    xWallPos1 = 0.0f;
		    xWallPos2 = -0.2f;
		    xWallPos3 = -0.2f;
		    zWallPos1 = 0.4f;
		    zWallPos2 = 0.4f;
		    zWallPos3 = 0.0f;

            xNormFront = 1.0f;
            xNormBack  = -1.0f;
            zNormFront = 0.0f;
            zNormBack  = 0.0f;
        }
        else
        {
            xWallOffset = 0.0f;
		    zWallOffset = 0.6f;
		    xWallWidth = 0.4f;
		    zWallWidth = -0.2f;
		    
		    xWallPos1 = 0.4f;
		    xWallPos2 = 0.0f;
		    xWallPos3 = 0.6f;
		    zWallPos1 = 0.0f;
		    zWallPos2 = -0.2f;
		    zWallPos3 = -0.2f;

            xNormFront = 0.0f;
            xNormBack  = 0.0f;
            zNormFront = 1.0f;
            zNormBack  = -1.0f;
        }
    }


    // Add in the partial floor area
    // Bottom left corner of tile (vertex 0)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top left corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top right corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Top right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + (zFloorOffset + zFloorWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - (zFloorOffset + zFloorWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xFloorOffset + xFloorWidth), m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom left corner of tile (vertex 1)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xFloorOffset, m_gameWorldGrid[tileIndex].y, m_gameWorldGrid[tileIndex].z + zFloorOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xFloorOffset, m_gameWorldGrid[tileIndex].tv - zFloorOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Add in the top of the wall
    // Bottom left corner of tile (vertex 0)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].tv - zWallOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top left corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].tv - (zWallOffset + zWallWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Top right corner of tile (vertex 0)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWallOffset, m_gameWorldGrid[tileIndex].tv - (zWallOffset + zWallWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;


	// Top right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallWidth));
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWallOffset, m_gameWorldGrid[tileIndex].tv - (zWallOffset + zWallWidth));
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom right corner of tile (vertex 1)
	currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + xWallOffset, m_gameWorldGrid[tileIndex].tv - zWallOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

	// Bottom left corner of tile (vertex 1)
    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].y + 3.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + (xWallOffset + xWallWidth), m_gameWorldGrid[tileIndex].tv - zWallOffset);
    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
	vertices.push_back(currVertex);
	indices.push_back(index);
	index++;

    // Wall is 3m high
    for (j = 0; j < 3; j++)
    {
        float yPos = m_gameWorldGrid[tileIndex].y + ((float)j*1.0f);


        // Bottom left corner of tile (vertex 0)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos1), yPos, m_gameWorldGrid[tileIndex].z + zWallOffset);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos1), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + zWallOffset);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 0)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos1));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos1));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 1)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xWallOffset, yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos1));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 1)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos1), yPos, m_gameWorldGrid[tileIndex].z + zWallOffset);
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormFront, m_gameWorldGrid[tileIndex].ny, zNormFront);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;


        // Other side of the wall
		// Bottom left corner of tile (vertex 2)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos2), yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos2));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top left corner of tile (vertex 2)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos2), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos2));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Top right corner of tile (vertex 2)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos3), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos3));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;


		// Top right corner of tile (vertex 3)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos3), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos3));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom right corner of tile (vertex 3)
		currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos3), yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos3));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;

		// Bottom left corner of tile (vertex 3)
        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xWallOffset + xWallPos2), yPos, m_gameWorldGrid[tileIndex].z + (zWallOffset + zWallPos2));
        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
	    currVertex.normal = D3DXVECTOR3(xNormBack, m_gameWorldGrid[tileIndex].ny, zNormBack);
		currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
		vertices.push_back(currVertex);
	    indices.push_back(index);
		index++;
    }
}


void GameWorld::AddTileFloorGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                                 std::vector<unsigned long> &indices, uint32 &index)
{
    gameWorldVertex_t currVertex;
    
    float xFloorOffset;
    float zFloorOffset;
    float xFloorWidth;
    float zFloorWidth;
	
}


void GameWorld::AddTileWallGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                                std::vector<unsigned long> &indices, uint32 &index)
{
    gameWorldVertex_t currVertex;
    int j;

    float xWall1Offset = 0.6f;
    float zWall1Offset = 0.4f;
    float xWall1Width = -0.2f;
    float zWall1Width = 0.6f;

    float xWall2Offset = 0.4f;
    float zWall2Offset = 0.4f;
    float xWall2Width = 0.6f;
    float zWall2Width = 0.2f;
    
    float xNormOffset = -2.0f;
    float zNormOffset = -2.0f;	
}


bool GameWorld::InitializeBuffers(ID3D11Device* device)
{
	//gameWorldVertex_t* vertices;
	std::vector<unsigned long> indices;
	uint32 index, i, j;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	//int index1, index2, index3, index4;
	//float tu, tv;


	// Calculate the number of vertices in the world mesh.
	//m_vertexCount = m_tileCount * 6;
	//m_vertexCount = (m_worldLength - 1) * (m_worldWidth - 1) * 6;
	//m_vertexCount += ((m_worldLength-1)*2)*6;

	// Set the index count to the same as the vertex count.
	//m_indexCount = m_vertexCount;

	// Create the vertex array.
	/*
	m_vertices = new std::vector<gameWorldVertex_t>;
	if(!m_vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new std::vector<unsigned long>;
	if(!indices)
	{
		return false;
	}
*/
	// Initialize the index to the vertex array.
	index = 0;

	gameWorldVertex_t currVertex;

	for (i = 0; i < m_tileCount; i++)
	{
        // If the tile is a floor tile
        if (m_gameWorldGrid[i].ny == 1.0f)
        {
	        // If it is a doorway, add in the door-jam walls
	        if (m_gameWorldGrid[i].nx == 2.0f)
	        {
                m_gameWorldGrid[i].nx = 0.0f;

                // Wall is 3m high
			    for (j = 0; j < 3; j++)
			    {
			        float yPos = m_gameWorldGrid[i].y + ((float)j*1.0f);

			        // Bottom left corner of tile (vertex 0)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, yPos, m_gameWorldGrid[i].z + 0.4f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top left corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, yPos + 1.0f, m_gameWorldGrid[i].z + 0.4f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top right corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, yPos + 1.0f, m_gameWorldGrid[i].z + 0.6f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

                    // Top right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, yPos + 1.0f, m_gameWorldGrid[i].z + 0.6f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, yPos, m_gameWorldGrid[i].z + 0.6f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom left corner of tile (vertex 1)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, yPos, m_gameWorldGrid[i].z + 0.4f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;


                    // Opposite Door-Jam
					// Bottom left corner of tile (vertex 0)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, yPos, m_gameWorldGrid[i].z + 0.6f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top left corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, yPos + 1.0f, m_gameWorldGrid[i].z + 0.6f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top right corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, yPos + 1.0f, m_gameWorldGrid[i].z + 0.4f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

                    // Top right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, yPos + 1.0f, m_gameWorldGrid[i].z + 0.4f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, yPos, m_gameWorldGrid[i].z + 0.4f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom left corner of tile (vertex 1)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, yPos, m_gameWorldGrid[i].z + 0.6f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;
			    }

	        }
	        else if (m_gameWorldGrid[i].nz == 2.0f)
	        {
                m_gameWorldGrid[i].nz = 0.0f;

                // Wall is 3m high
			    for (j = 0; j < 3; j++)
			    {
			        float yPos = m_gameWorldGrid[i].y + ((float)j*1.0f);

			        // Bottom left corner of tile (vertex 0)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.6f, yPos, m_gameWorldGrid[i].z);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top left corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.6f, yPos + 1.0f, m_gameWorldGrid[i].z);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top right corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.4f, yPos + 1.0f, m_gameWorldGrid[i].z);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

                    // Top right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.4f, yPos + 1.0f, m_gameWorldGrid[i].z);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.4f, yPos, m_gameWorldGrid[i].z);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom left corner of tile (vertex 1)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.6f, yPos, m_gameWorldGrid[i].z);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;


                    // Opposite Door-Jam
					// Bottom left corner of tile (vertex 0)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.4f, yPos, m_gameWorldGrid[i].z + 1.0f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top left corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.4f, yPos + 1.0f, m_gameWorldGrid[i].z + 1.0f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Top right corner of tile (vertex 0)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.6f, yPos + 1.0f, m_gameWorldGrid[i].z + 1.0f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

                    // Top right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.6f, yPos + 1.0f, m_gameWorldGrid[i].z + 1.0f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom right corner of tile (vertex 1)
					currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.6f, yPos, m_gameWorldGrid[i].z + 1.0f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.6f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;

					// Bottom left corner of tile (vertex 1)
			        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.4f, yPos, m_gameWorldGrid[i].z + 1.0f);
			        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 0.4f);
				    currVertex.normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
					currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
					m_vertices.push_back(currVertex);
				    indices.push_back(index);
					index++;
			    }
	        }

	        // Bottom left corner of tile (vertex 0)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[i].nx, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top left corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 1);
		    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[i].nx, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top right corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1, m_gameWorldGrid[i].tv - 1);
		    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[i].nx, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;


			// Top right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1, m_gameWorldGrid[i].tv - 1);
		    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[i].nx, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[i].nx, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom left corner of tile (vertex 1)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(m_gameWorldGrid[i].nx, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;
		}
        // Check if the current tile is a corner and if so, add the vertices to the list
        else if (abs(m_gameWorldGrid[i].nx) == 1.0f && abs(m_gameWorldGrid[i].nz) == 1.0f)
        {
        	// This is a corner tile, so send that info the the wall geometry function
        	CreateWallCornerGeometry(i, m_vertices, indices, index);
        }
        // If the current tile is a Wall, then add the wall geometry vertices to the list
		else if (m_gameWorldGrid[i].nx != 0.0f || m_gameWorldGrid[i].nz != 0.0f)
		{
		    // This is a standard wall tile
		    CreateWallGeometry(i, m_vertices, indices, index);

		    // Now check to see if any half-wall sections are needed by looking at the r,g,b values of that tile
		    if (m_gameWorldGrid[i].r != 0.0f && m_gameWorldGrid[i].b != 0.0f)
		    {
		    	CreateHalfWallGeometry(i, m_vertices, indices, index);
		    }
		}

/*
		    // Add in the partial floor area
		    // Bottom left corner of tile (vertex 0)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top left corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top right corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;


			// Top right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 1.0f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom left corner of tile (vertex 1)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

		    // Wall is 3m high
		    for (j = 0; j < 3; j++)
		    {
                float yPos = m_gameWorldGrid[i].y + ((float)j*1.0f);

                // Bottom left corner of tile (vertex 0)
		        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top left corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top right corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;


				// Top right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom left corner of tile (vertex 1)
		        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;
		    }
		 }
*/

/*
		// If it is a wall facing west
		if (m_gameWorldGrid[i].nx == -1.0f && m_gameWorldGrid[i].nz == 0.0f)
		{
		    // Add in the partial floor area
		    // Bottom left corner of tile (vertex 0)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top left corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top right corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;


			// Top right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom left corner of tile (vertex 1)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

		    // Wall is 3m high
		    for (j = 0; j < 3; j++)
		    {
                float yPos = m_gameWorldGrid[i].y + ((float)j*1.0f);

                // Bottom left corner of tile (vertex 0)
		        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top left corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top right corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;


				// Top right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom left corner of tile (vertex 1)
		        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;
		    }		
		}

		// If it is a wall facing north
		if (m_gameWorldGrid[i].nx == 0.0f && m_gameWorldGrid[i].nz == 1.0f)
		{
		    // Add in the partial floor area
		    // Bottom left corner of tile (vertex 0)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top left corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Top right corner of tile (vertex 0)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;


			// Top right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z + 1.0f);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv - 1.0f);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom right corner of tile (vertex 1)
			currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 0.5f, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

			// Bottom left corner of tile (vertex 1)
	        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x, m_gameWorldGrid[i].y, m_gameWorldGrid[i].z);
	        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
		    currVertex.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
			m_vertices.push_back(currVertex);
			indices.push_back(index);
			index++;

		    // Wall is 3m high
		    for (j = 0; j < 3; j++)
		    {
                float yPos = m_gameWorldGrid[i].y + ((float)j*1.0f);

                // Bottom left corner of tile (vertex 0)
		        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top left corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top right corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;


				// Top right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos + 1.0f, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu + 1.0f, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom left corner of tile (vertex 1)
		        currVertex.position = D3DXVECTOR3(m_gameWorldGrid[i].x + 0.5f, yPos, m_gameWorldGrid[i].z + 1.0f);
		        currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[i].tu, m_gameWorldGrid[i].tv);
			    currVertex.normal = D3DXVECTOR3(-1.0f, m_gameWorldGrid[i].ny, m_gameWorldGrid[i].nz);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[i].r, m_gameWorldGrid[i].g, m_gameWorldGrid[i].b, 1.0f);
				m_vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;
		    }		
		}
*/
	}
	
	m_vertexCount = (int)m_vertices.size();
	m_indexCount = (int)indices.size();

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(gameWorldVertex_t) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = &(m_vertices[0]);
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = &(indices[0]);
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the buffers have been created and loaded.
	//delete [] vertices;
	//vertices = nullptr;

	return true;
}


void GameWorld::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
/*
	// Release the game world vertices.
	if(m_vertices)
	{
		delete m_vertices;
	    m_vertices = nullptr;
	}
*/
	return;
}


void GameWorld::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(gameWorldVertex_t); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


//--------------------------------------------
// Procedurally Generated GameWorld
//--------------------------------------------
bool GameWorld::GenerateRandomWorld(int maxFeatures)
{
	bool result = true;
	m_procWorldLength = 96;
	m_procWorldWidth = 96;
	m_tiles = std::vector<char>(m_procWorldLength*m_procWorldWidth, Unused);
	ofstream fout;

/*	
	m_tiles = new std::vector<char>(m_procWorldLength*m_procWorldWidth, Unused);
	if (!m_tiles)
	{
		return false;
	}

    m_rooms = new std::vector<Rect>();
    if (!m_rooms)
	{
		return false;
	}

	m_exits = new std::vector<Rect>();
	if (!m_exits)
	{
		return false;
	}
*/

	// Open a file to write the error message to.
	fout.open("world-gen-error.txt");

	// place the first room in the center
	if (!MakeRoom(m_procWorldLength / 2, m_procWorldWidth / 2, static_cast<Direction>(Gumshoe::RandomInt(4)), true))
	{
		fout << "Unable to place the first room.\n";
		result = false;
	}

	// we already placed 1 feature (the first room)
	for (int i = 1; i < maxFeatures; i++)
	{
		if (!CreateFeature())
		{
			fout << "Unable to place more features (placed " << i << ").\n";
			break;
		}
	}

	if (!PlaceObject(UpStairs))
	{
		fout << "Unable to place up stairs.\n";	
		result = false;
	}

	if (!PlaceObject(DownStairs))
	{
		fout << "Unable to place down stairs.\n";		
		result = false;
	}

	// Replace all unused tiles with '.' and set the rooms and corridors to ' '
	m_tileCount = 0;
	for (char& tile : m_tiles)
	{
		if (tile == Unused)
			tile = '.';
		else if (tile == Floor || tile == Corridor)
		{
			tile = ' ';
			m_tileCount++;
		}
		else
		{
            m_tileCount++;
		}
	}

    // Close the file.
	fout.close();

	return result;
}


void GameWorld::PrintWorld()
{
	ofstream fout;

	// Open the output map file
	fout.open("game_world.txt");

	for (int y = m_procWorldWidth-1; y >= 0; --y)
	{
		for (int x = 0; x < m_procWorldLength; ++x)
			fout << GetTile(x, y);

		fout << std::endl;
	}

	// Close the file.
	fout.close();
}


char GameWorld::GetTile(int x, int y)
{
	if (x < 0 || y < 0 || x >= m_procWorldLength || y >= m_procWorldWidth)
		return Unused;

	return m_tiles[x + y * m_procWorldLength];
}


void GameWorld::SetTile(int x, int y, char tile)
{
    m_tiles[x + y * m_procWorldLength] = tile;
}


bool GameWorld::CreateFeature()
{
    for (int i = 0; i < 1000; i++)
	{
		if (m_exits.empty())
			break;

		// choose a random side of a random room or corridor
		int r = Gumshoe::RandomInt((int)m_exits.size());
		int x = Gumshoe::RandomInt(m_exits[r].x, m_exits[r].x + m_exits[r].xSize - 1);
		int y = Gumshoe::RandomInt(m_exits[r].y, m_exits[r].y + m_exits[r].ySize - 1);

		// north, south, west, east
		for (int j = 0; j < DirectionCount; ++j)
		{
			if (CreateFeature(x, y, static_cast<Direction>(j)))
			{
				m_exits.erase(m_exits.begin() + r);
				return true;
			}
		}
	}

	return false;
}

bool GameWorld::CreateFeature(int x, int y, Direction dir)
{
	static const int roomChance = 50; // corridorChance = 100 - roomChance

	int dx = 0;
	int dy = 0;

	if (dir == North)
		dy = 1;
	else if (dir == South)
		dy = -1;
	else if (dir == West)
		dx = 1;
	else if (dir == East)
		dx = -1;

	if (GetTile(x + dx, y + dy) != Floor && GetTile(x + dx, y + dy) != Corridor)
		return false;

	if (Gumshoe::RandomInt(100) < roomChance)
	{
		if (MakeRoom(x, y, dir, false))
		{
			SetTile(x, y, ClosedDoor);
			return true;
		}
	}
    else
	{
		if (MakeCorridor(x, y, dir))
		{
			if (GetTile(x + dx, y + dy) == Floor)
				SetTile(x, y, ClosedDoor);
			else // don't place a door between corridors
				SetTile(x, y, Corridor);
			return true;
		}
	}

	return false;
}


bool GameWorld::MakeRoom(int x, int y, Direction dir, bool firstRoom)
{
	static const int minRoomSize = 3;
	static const int maxRoomSize = 6;

	Rect room;
	room.xSize = Gumshoe::RandomInt(minRoomSize, maxRoomSize);
	room.ySize = Gumshoe::RandomInt(minRoomSize, maxRoomSize);

	if (dir == North)
	{
		room.x = x - room.xSize / 2;
		room.y = y - room.ySize;
	}

	else if (dir == South)
	{
		room.x = x - room.xSize / 2;
		room.y = y + 1;
	}

	else if (dir == West)
	{
		room.x = x - room.xSize;
		room.y = y - room.ySize / 2;
	}

	else if (dir == East)
	{
		room.x = x + 1;
		room.y = y - room.ySize / 2;
	}

	if (PlaceRect(room, Floor))
	{
		m_rooms.push_back(room);

		// Put exits in the room
		if (dir != South || firstRoom) // north side
			m_exits.push_back(Rect{ room.x, room.y - 1, room.xSize, 1 });
		if (dir != North || firstRoom) // south side
			m_exits.push_back(Rect{ room.x, room.y + room.ySize, room.xSize, 1 });
		if (dir != East || firstRoom)  // west side
			m_exits.push_back(Rect{ room.x - 1, room.y, 1, room.ySize });
		if (dir != West || firstRoom)  // east side
			m_exits.push_back(Rect{ room.x + room.xSize, room.y, 1, room.ySize });

		return true;
	}

	return false;
}


bool GameWorld::MakeCorridor(int x, int y, Direction dir)
{
	static const int minCorridorLength = 3;
	static const int maxCorridorLength = 6;

	Rect corridor;
	corridor.x = x;
	corridor.y = y;

	if (Gumshoe::RandomBool()) // horizontal corridor
	{
		corridor.xSize = Gumshoe::RandomInt(minCorridorLength, maxCorridorLength);
		corridor.ySize = 1;

		if (dir == North)
		{
			corridor.y = y - 1;

			if (Gumshoe::RandomBool()) // west
				corridor.x = x - corridor.xSize + 1;
		}

		else if (dir == South)
		{
			corridor.y = y + 1;

			if (Gumshoe::RandomBool()) // west
				corridor.x = x - corridor.xSize + 1;
		}

		else if (dir == West)
			corridor.x = x - corridor.xSize;

		else if (dir == East)
			corridor.x = x + 1;
	}

	else // vertical corridor
	{
		corridor.xSize = 1;
		corridor.ySize = Gumshoe::RandomInt(minCorridorLength, maxCorridorLength);

		if (dir == North)
			corridor.y = y - corridor.ySize;

		else if (dir == South)
			corridor.y = y + 1;

		else if (dir == West)
		{
			corridor.x = x - 1;

			if (Gumshoe::RandomBool()) // north
				corridor.y = y - corridor.ySize + 1;
		}

		else if (dir == East)
		{
			corridor.x = x + 1;

			if (Gumshoe::RandomBool()) // north
				corridor.y = y - corridor.ySize + 1;
		}
	}

	if (PlaceRect(corridor, Corridor))
	{
		if (dir != South && corridor.xSize != 1) // north side
			m_exits.emplace_back(Rect{ corridor.x, corridor.y - 1, corridor.xSize, 1 });
		if (dir != North && corridor.xSize != 1) // south side
			m_exits.emplace_back(Rect{ corridor.x, corridor.y + corridor.ySize, corridor.xSize, 1 });
		if (dir != East && corridor.ySize != 1)   // west side
			m_exits.emplace_back(Rect{ corridor.x - 1, corridor.y, 1, corridor.ySize });
		if (dir != West && corridor.ySize != 1)   // east side
			m_exits.emplace_back(Rect{ corridor.x + corridor.xSize, corridor.y, 1, corridor.ySize });

		return true;
	}

	return false;
}


bool GameWorld::PlaceRect(const Rect& rect, char tile)
{
	if (rect.x < 1 || rect.y < 1 || rect.x + rect.xSize >= m_procWorldLength - 1 || rect.y + rect.ySize >= m_procWorldWidth - 1)
		return false;

    // Loop through the current Rect tiles to make sure it is all in unused space
	for (int y = rect.y; y < rect.y + rect.ySize; ++y)
		for (int x = rect.x; x < rect.x + rect.xSize; ++x)
		{
			if (GetTile(x, y) != Unused)
				return false; // the area already used
		}

	// Now place the Rect, putting walls at the edges
	for (int y = rect.y - 1; y < rect.y + rect.ySize + 1; ++y)
		for (int x = rect.x - 1; x < rect.x + rect.xSize + 1; ++x)
		{
			if (x == rect.x - 1 || y == rect.y - 1 || x == rect.x + rect.xSize || y == rect.y + rect.ySize)
				SetTile(x, y, Wall);
			else
				SetTile(x, y, tile);
		}

	return true;
}


bool GameWorld::PlaceObject(char tile)
{
	if (m_rooms.empty())
		return false;

	int r = Gumshoe::RandomInt((int)m_rooms.size()); // choose a random room
	int x = Gumshoe::RandomInt(m_rooms[r].x + 1, m_rooms[r].x + m_rooms[r].xSize - 2);
	int y = Gumshoe::RandomInt(m_rooms[r].y + 1, m_rooms[r].y + m_rooms[r].ySize - 2);

	if (GetTile(x, y) == Floor)
	{
		SetTile(x, y, tile);

		// place one object in one room (optional)
		m_rooms.erase(m_rooms.begin() + r);

		return true;
	}

	return false;
}
