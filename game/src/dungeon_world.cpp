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

          	// Check each adjacent tile to set the floor features
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

            // Check the North tile to add walls
            if (northTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= WestWall;
                m_gameWorldGrid[index].geoFeatures |= EastWall;
                if (eastTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~EastWall;
                if (westTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~WestWall;
            }
            else if (northTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthWall;
                m_gameWorldGrid[index].geoFeatures |= NorthWallCap;
            }
            else if (northTile == Wall &&
            	    ((m_gameWorldGrid[index].geoFeatures & NorthEastFloor) || 
            	     (m_gameWorldGrid[index].geoFeatures & NorthWestFloor)))
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthWall;
            }

            // Check the South tile to add walls
            if (southTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= WestWall;
                m_gameWorldGrid[index].geoFeatures |= EastWall;
                if (eastTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~EastWall;
                if (westTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~WestWall;
            }
            else if (southTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthWall;
                m_gameWorldGrid[index].geoFeatures |= SouthWallCap;
            }
            else if (southTile == Wall &&
            	    ((m_gameWorldGrid[index].geoFeatures & SouthEastFloor) || 
            	     (m_gameWorldGrid[index].geoFeatures & SouthWestFloor)))
            {
            	m_gameWorldGrid[index].geoFeatures |= SouthWall;
            }

          	// Check the East tile to add walls
            if (eastTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthWall;
                m_gameWorldGrid[index].geoFeatures |= SouthWall;
                if (northTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~NorthWall;
                if (southTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~SouthWall;
            }
            else if (eastTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
                m_gameWorldGrid[index].geoFeatures |= EastWall;
                m_gameWorldGrid[index].geoFeatures |= EastWallCap;
            }
            else if (eastTile == Wall &&
            	    ((m_gameWorldGrid[index].geoFeatures & NorthEastFloor) || 
            	     (m_gameWorldGrid[index].geoFeatures & SouthEastFloor)))
            {
            	m_gameWorldGrid[index].geoFeatures |= EastWall;
            }

            // Check the West tile to add walls
            if (westTile == ' ')
            {
                m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= NorthWall;
                m_gameWorldGrid[index].geoFeatures |= SouthWall;
                if (northTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~NorthWall;
                if (southTile == ' ')
                	m_gameWorldGrid[index].geoFeatures &= ~SouthWall;
            }
            else if (westTile == ClosedDoor)
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
                m_gameWorldGrid[index].geoFeatures |= WestWall;
                m_gameWorldGrid[index].geoFeatures |= WestWallCap;
            }
            else if (westTile == Wall &&
            	    ((m_gameWorldGrid[index].geoFeatures & NorthWestFloor) || 
            	     (m_gameWorldGrid[index].geoFeatures & SouthWestFloor)))
            {
            	m_gameWorldGrid[index].geoFeatures |= WestWall;
            }


            // Check to add in the walls that just come to an end and don't connect
            if (northTile == ' ' && southTile == Wall && eastTile == ' ' && westTile == ' ')
            {
            	m_gameWorldGrid[index].geoFeatures |= NorthWall;
            }

            if (northTile == Wall && southTile == ' ' && eastTile == ' ' && westTile == ' ')
            {
            	m_gameWorldGrid[index].geoFeatures |= SouthWall;
            }

            if (northTile == ' ' && southTile == ' ' && eastTile == Wall && westTile == ' ')
            {
            	m_gameWorldGrid[index].geoFeatures |= WestWall;
            }

            if (northTile == ' ' && southTile == ' ' && eastTile == ' ' && westTile == Wall)
            {
            	m_gameWorldGrid[index].geoFeatures |= EastWall;
            }


            // Lastly, check if any walls need an end cap
            if ((m_gameWorldGrid[index].geoFeatures & NorthWall) && (northTile != Wall))
                m_gameWorldGrid[index].geoFeatures |= NorthWallCap;

            if ((m_gameWorldGrid[index].geoFeatures & SouthWall) && (southTile != Wall))
                m_gameWorldGrid[index].geoFeatures |= SouthWallCap;

            if ((m_gameWorldGrid[index].geoFeatures & EastWall) && (eastTile != Wall))
                m_gameWorldGrid[index].geoFeatures |= EastWallCap;

            if ((m_gameWorldGrid[index].geoFeatures & WestWall) && (westTile != Wall))
                m_gameWorldGrid[index].geoFeatures |= WestWallCap;

            // Set the y-direction normal to 0
            m_gameWorldGrid[index].ny = 0.0f;
          }
          else if (tile == ClosedDoor)
          {
          	m_gameWorldGrid[index].geoFeatures |= NorthWestFloor;
          	m_gameWorldGrid[index].geoFeatures |= NorthEastFloor;
          	m_gameWorldGrid[index].geoFeatures |= SouthWestFloor;
          	m_gameWorldGrid[index].geoFeatures |= SouthEastFloor;
          	m_gameWorldGrid[index].geoFeatures |= Doorway;
          	m_gameWorldGrid[index].ny = 1.0f;
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


void GameWorld::AddTileFloorGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                                 std::vector<unsigned long> &indices, uint32 &index)
{
    gameWorldVertex_t currVertex;
    int j;
    bool wholeFloor = false;
    
    float xFloorOffset;
    float zFloorOffset;
    float xFloorWidth;
    float zFloorWidth;

    // Check if the whole tile should have a floor, to save polygons
    if ((m_gameWorldGrid[tileIndex].geoFeatures & (NorthWestFloor | NorthEastFloor | SouthWestFloor | SouthEastFloor)) == 0x0F)
    {
    	xFloorOffset = 0.0f;
    	zFloorOffset = 0.0f;
    	xFloorWidth  = 1.0f;
    	zFloorWidth  = 1.0f;
    	j = 3;
    	wholeFloor = true;
    }
    else
    {
    	xFloorOffset = 0.0f;
    	zFloorOffset = 0.0f;
    	xFloorWidth  = 0.5f;
    	zFloorWidth  = 0.5f;
    	j = 0;
    }

    for (; j < 4; j++)
    {
    	bool addFloor = false;
    	if (wholeFloor)
    	{
    		addFloor = true;
    	}
	    else if (m_gameWorldGrid[tileIndex].geoFeatures & (1<<j))
	    {
	    	addFloor = true;
	    	// xFloorOffset = ((float)(j % 2)) * 0.5f;
	    	// zFloorOffset = ((float)(((j+2) & 0x02)>>1)) * 0.5f;

	    	if (j == 0)
	    	{
                xFloorOffset = 0.0f;
                zFloorOffset = 0.5f;
	    	}
	    	else if (j == 1)
	    	{
                xFloorOffset = 0.5f;
                zFloorOffset = 0.5f;
	    	}
	    	else if (j == 2)
	    	{
                xFloorOffset = 0.0f;
                zFloorOffset = 0.0f;
	    	}
	    	else
	    	{
                xFloorOffset = 0.5f;
                zFloorOffset = 0.0f;
	    	}
	    }

	    if (addFloor)
	    {
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
		}
	}
}


void GameWorld::AddTileWallGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                                std::vector<unsigned long> &indices, uint32 &index)
{
    gameWorldVertex_t currVertex;
    int i, j;
    bool addWall = false;

    float xWallOffset = 0.6f;
    float zWallOffset = 0.4f;
    float xWallWidth = -0.2f;
    float zWallWidth = 0.6f;
    
    float xWallPos1 = 0.0f;
    float xWallPos2 = -0.2f;
    float xWallPos3 = -0.2f;
    float zWallPos1 = 0.6f;
    float zWallPos2 = 0.6f;
    float zWallPos3 = 0.0f;
   
    float xNormFront = 1.0f;
    float xNormBack = -1.0f;
    float zNormFront = 0.0f;
    float zNormBack = 0.0f;

    for (i = 0; i < 4; i++)
    {
    	addWall = false;

	    if (m_gameWorldGrid[tileIndex].geoFeatures & (1<<(i+4)))
	    {
	        addWall = true;
	        
	        if (i == 1)
	        {
                if (m_gameWorldGrid[tileIndex].geoFeatures & NorthWall)
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
                }
                else
                {
                    xWallOffset = 0.6f;
	                zWallOffset = 0.0f;
	                xWallWidth = -0.2f;
	                zWallWidth = 0.6f;

	                xWallPos1 = 0.0f;
				    xWallPos2 = -0.2f;
				    xWallPos3 = -0.2f;
				    zWallPos1 = 0.6f;
				    zWallPos2 = 0.6f;
				    zWallPos3 = 0.0f;	
                }

                xNormFront = 1.0f;
                xNormBack = -1.0f;
                zNormFront = 0.0f;
                zNormBack = 0.0f;
	        }
	        else if (i == 2)
	        {
                xWallOffset = 0.4f;
                zWallOffset = 0.6f;
                xWallWidth = 0.6f;
                zWallWidth = -0.2f;

                xWallPos1 = 0.6f;
			    xWallPos2 = 0.0f;
			    xWallPos3 = 0.6f;
			    zWallPos1 = 0.0f;
			    zWallPos2 = -0.2f;
			    zWallPos3 = -0.2f;

                xNormFront = 0.0f;
                xNormBack = 0.0f;
                zNormFront = 1.0f;
                zNormBack = -1.0f;
	        }
	        else if (i == 3)
	        {
                if (m_gameWorldGrid[tileIndex].geoFeatures & EastWall)
                {
                    xWallOffset = 0.0f;
	                zWallOffset = 0.6f;
	                xWallWidth = 0.4f;
	                zWallWidth = -0.2f;

	                xWallPos1 = 0.4f;
				    xWallPos2 = 0.0f;
				    xWallPos3 = 0.4f;
				    zWallPos1 = 0.0f;
				    zWallPos2 = -0.2f;
				    zWallPos3 = -0.2f;
                }
                else
                {
                    xWallOffset = 0.0f;
	                zWallOffset = 0.6f;
	                xWallWidth = 0.6f;
	                zWallWidth = -0.2f;

	                xWallPos1 = 0.6f;
				    xWallPos2 = 0.0f;
				    xWallPos3 = 0.6f;
				    zWallPos1 = 0.0f;
				    zWallPos2 = -0.2f;
				    zWallPos3 = -0.2f;	
                }

                xNormFront = 0.0f;
                xNormBack = 0.0f;
                zNormFront = 1.0f;
                zNormBack = -1.0f;
	        }
	    }

	    if (addWall)
	    {
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
	}
}


void GameWorld::AddTileWallCapGeometry(uint32 tileIndex, std::vector<gameWorldVertex_t> &vertices,
	                                   std::vector<unsigned long> &indices, uint32 &index)
{
	gameWorldVertex_t currVertex;
    int i, j;

    float xCapOffset = 0.6f;
    float zCapOffset = 1.0f;
    float xCapWidth = -0.2f;
    float zCapWidth = 0.0f;
   
    float xNorm = 0.0f;
    float zNorm = 1.0f;

    for (i = 0; i < 4; i++)
    {
    	if (m_gameWorldGrid[tileIndex].geoFeatures & (1<<(i+8)))
	    {
	        if (i == 1)
	        {
	        	xCapOffset = 0.4f;
	        	zCapOffset = 0.0f;
	        	xCapWidth  = 0.2f;
	        	zCapWidth  = 0.0f;
	        	xNorm = 0.0f;
	        	zNorm = -1.0f;
	        }
	        else if (i == 2)
	        {
                xCapOffset = 1.0f;
	        	zCapOffset = 0.4f;
	        	xCapWidth  = 0.0f;
	        	zCapWidth  = 0.2f;
	        	xNorm = 1.0f;
	        	zNorm = 0.0f;
	        }
	        else if (i == 3)
	        {
                xCapOffset = 0.0f;
	        	zCapOffset = 0.6f;
	        	xCapWidth  = 0.0f;
	        	zCapWidth  = -0.2f;
	        	xNorm = -1.0f;
	        	zNorm = 0.0f;
	        }
 
            // Wall is 3m high
		    for (j = 0; j < 3; j++)
		    {
		        float yPos = m_gameWorldGrid[tileIndex].y + ((float)j*1.0f);
            
	            // Bottom left corner of tile (vertex 0)
			    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xCapOffset, yPos, m_gameWorldGrid[tileIndex].z + zCapOffset);
			    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
			    currVertex.normal = D3DXVECTOR3(xNorm, m_gameWorldGrid[tileIndex].ny, zNorm);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top left corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xCapOffset, yPos + 1.0f, m_gameWorldGrid[tileIndex].z + zCapOffset);
			    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(xNorm, m_gameWorldGrid[tileIndex].ny, zNorm);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Top right corner of tile (vertex 0)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xCapOffset + xCapWidth), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zCapOffset + zCapWidth));
			    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(xNorm, m_gameWorldGrid[tileIndex].ny, zNorm);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;


				// Top right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xCapOffset + xCapWidth), yPos + 1.0f, m_gameWorldGrid[tileIndex].z + (zCapOffset + zCapWidth));
			    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv - 1.0f);
			    currVertex.normal = D3DXVECTOR3(xNorm, m_gameWorldGrid[tileIndex].ny, zNorm);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom right corner of tile (vertex 1)
				currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + (xCapOffset + xCapWidth), yPos, m_gameWorldGrid[tileIndex].z + (zCapOffset + zCapWidth));
			    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu + 1.0f, m_gameWorldGrid[tileIndex].tv);
			    currVertex.normal = D3DXVECTOR3(xNorm, m_gameWorldGrid[tileIndex].ny, zNorm);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;

				// Bottom left corner of tile (vertex 1)
			    currVertex.position = D3DXVECTOR3(m_gameWorldGrid[tileIndex].x + xCapOffset, yPos, m_gameWorldGrid[tileIndex].z + zCapOffset);
			    currVertex.texture = D3DXVECTOR2(m_gameWorldGrid[tileIndex].tu, m_gameWorldGrid[tileIndex].tv);
			    currVertex.normal = D3DXVECTOR3(xNorm, m_gameWorldGrid[tileIndex].ny, zNorm);
				currVertex.color = D3DXVECTOR4(m_gameWorldGrid[tileIndex].r, m_gameWorldGrid[tileIndex].g, m_gameWorldGrid[tileIndex].b, 1.0f);
				vertices.push_back(currVertex);
			    indices.push_back(index);
				index++;
			}
		}
	}
}


bool GameWorld::InitializeBuffers(ID3D11Device* device)
{
	//gameWorldVertex_t* vertices;
	std::vector<unsigned long> indices;
	uint32 index, i;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Initialize the index to the vertex array.
	index = 0;

	gameWorldVertex_t currVertex;

	for (i = 0; i < m_tileCount; i++)
	{
        // First add the floor polygons for the tile
        AddTileFloorGeometry(i, m_vertices, indices, index);

        // Then add the wall polygons for the tile
        AddTileWallGeometry(i, m_vertices, indices, index);

        // Then add the wall-cap polygons for the tile
        AddTileWallCapGeometry(i, m_vertices, indices, index);
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
