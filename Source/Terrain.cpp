#include "Terrain.h"

Terrain::Terrain()
{
	_vertexBuffer = 0;
	_indexBuffer = 0;
	_terrainFilename = 0;
	_heightMap = 0;
	_terrainModel = 0;
}

Terrain::Terrain(const Terrain& other)
{
}

Terrain::~Terrain()
{
}

bool Terrain::Initialize(ID3D11Device* device, char* setupFilename)
{
	bool result;

	// Get the terrain filename, dimensions, and so forth from the setup file.
	result = LoadSetupFile(setupFilename);
	if (!result)
	{
		return false;
	}

	// Initialize the terrain height map with the data from the bitmap file.
	result = LoadBitmapHeightMap();
	if (!result)
	{
		return false;
	}

	// Setup the X and Z coordinates for the height map as well as scale the terrain height by the height scale value.
	SetTerrainCoordinates();

	// Calculate the normals for the terrain data.
	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	// Now build the 3D model of the terrain.
	result = BuildTerrainModel();
	if (!result)
	{
		return false;
	}

	// We can now release the height map since it is no longer needed in memory once the 3D terrain model has been built.
	DestroyHeightMap();

	// Load the rendering buffers with the terrain data.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	// Release the terrain model now that the rendering buffers have been loaded.
	DestroyTerrainModel();

	return true;
}

void Terrain::Destroy()
{
	// Release the rendering buffers.
	DestroyBuffers();

	// Release the terrain model.
	DestroyTerrainModel();

	// Release the height map.
	DestroyHeightMap();

	return;
}

bool Terrain::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return true;
}

int Terrain::GetIndexCount()
{
	return _indexCount;
}

bool Terrain::LoadSetupFile(char * filename)
{
	int stringLength;
	ifstream fin;
	char input;

	// Initialize the string that will hold the terrain file name.
	stringLength = 256;
	_terrainFilename = new char[stringLength];
	if (!_terrainFilename)
	{
		return false;
	}

	// Open the setup file.  If it could not open the file then exit.
	fin.open(filename);
	if (fin.fail())
	{
		return false;
	}

	// Read up to the terrain file name.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain file name.
	fin >> _terrainFilename;

	// Read up to the value of terrain height.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain height.
	fin >> _terrainHeight;

	// Read up to the value of terrain width.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain width.
	fin >> _terrainWidth;

	// Read up to the value of terrain height scaling.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the terrain height scaling.
	fin >> _heightScale;

	// Close the setup file.
	fin.close();

	return true;
}

bool Terrain::LoadBitmapHeightMap()
{
	int error, imageSize, i, j, k, index;
	FILE* filePtr;
	unsigned long long count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char* bitmapImage;
	unsigned char height;

	// Start by creating the array structure to hold the height map data.
	_heightMap = new HeightMapType[_terrainWidth * _terrainHeight];
	if (!_heightMap)
	{
		return false;
	}

	// Open the bitmap map file in binary.
	error = fopen_s(&filePtr, _terrainFilename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the bitmap file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Make sure the height map dimensions are the same as the terrain dimensions for easy 1 to 1 mapping.
	if ((bitmapInfoHeader.biHeight != _terrainHeight) || (bitmapInfoHeader.biWidth != _terrainWidth))
	{
		return false;
	}

	// Calculate the size of the bitmap image data.  
	// Since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line.
	imageSize = _terrainHeight * ((_terrainWidth * 3) + 1);

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Initialize the Position in the image data buffer.
	k = 0;

	// Read the image data into the height map array.
	for (j = 0; j<_terrainHeight; j++)
	{
		for (i = 0; i<_terrainWidth; i++)
		{
			// Bitmaps are upside down so load bottom to top into the height map array.
			index = (_terrainWidth * (_terrainHeight - 1 - j)) + i;

			// Get the grey scale pixel value from the bitmap image data at this location.
			height = bitmapImage[k];

			// Store the pixel value as the height at this point in the height map array.
			_heightMap[index].Y = (float)height;

			// Increment the bitmap image data index.
			k += 3;
		}

		// Compensate for the extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257).
		k++;
	}

	// Release the bitmap image data now that the height map array has been loaded.
	delete[] bitmapImage;
	bitmapImage = 0;

	// Release the terrain filename now that is has been read in.
	delete[] _terrainFilename;
	_terrainFilename = 0;

	return true;
}

void Terrain::DestroyHeightMap()
{
	// Release the height map array.
	if (_heightMap)
	{
		delete[] _heightMap;
		_heightMap = 0;
	}

	return;
}

void Terrain::SetTerrainCoordinates()
{
	int i, j, index;

	// Loop through all the elements in the height map array and adjust their coordinates correctly.
	for (j = 0; j<_terrainHeight; j++)
	{
		for (i = 0; i<_terrainWidth; i++)
		{
			index = (_terrainWidth * j) + i;

			// Set the X and Z coordinates.
			_heightMap[index].X = (float)i;
			_heightMap[index].Z = -(float)j;

			// Move the terrain depth into the positive range.  For example from (0, -256) to (256, 0).
			_heightMap[index].Z += (float)(_terrainHeight - 1);

			// Scale the height.
			_heightMap[index].Y /= _heightScale;
		}
	}

	return;
}

bool Terrain::CalculateNormals()
{
	int i, j, index1, index2, index3, index;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	VectorType* normals;

	// Create a temporary array to hold the face normal vectors.
	normals = new VectorType[(_terrainHeight - 1) * (_terrainWidth - 1)];
	if (!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j<(_terrainHeight - 1); j++)
	{
		for (i = 0; i<(_terrainWidth - 1); i++)
		{
			index1 = ((j + 1) * _terrainWidth) + i;      // Bottom left vertex.
			index2 = ((j + 1) * _terrainWidth) + (i + 1);  // Bottom right vertex.
			index3 = (j * _terrainWidth) + i;          // Upper left vertex.

														// Get three vertices from the face.
			vertex1[0] = _heightMap[index1].X;
			vertex1[1] = _heightMap[index1].Y;
			vertex1[2] = _heightMap[index1].Z;

			vertex2[0] = _heightMap[index2].X;
			vertex2[1] = _heightMap[index2].Y;
			vertex2[2] = _heightMap[index2].Z;

			vertex3[0] = _heightMap[index3].X;
			vertex3[1] = _heightMap[index3].Y;
			vertex3[2] = _heightMap[index3].Z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (_terrainWidth - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].X = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].Y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].Z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);

			// Calculate the length.
			length = (float)sqrt((normals[index].X * normals[index].X) + (normals[index].Y * normals[index].Y) +
				(normals[index].Z * normals[index].Z));

			// Normalize the final value for this face using the length.
			normals[index].X = (normals[index].X / length);
			normals[index].Y = (normals[index].Y / length);
			normals[index].Z = (normals[index].Z / length);
		}
	}

	// Now go through all the vertices and take a sum of the face normals that touch this vertex.
	for (j = 0; j<_terrainHeight; j++)
	{
		for (i = 0; i<_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (_terrainWidth - 1)) + (i - 1);

				sum[0] += normals[index].X;
				sum[1] += normals[index].Y;
				sum[2] += normals[index].Z;
			}

			// Bottom right face.
			if ((i<(_terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (_terrainWidth - 1)) + i;

				sum[0] += normals[index].X;
				sum[1] += normals[index].Y;
				sum[2] += normals[index].Z;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j<(_terrainHeight - 1)))
			{
				index = (j * (_terrainWidth - 1)) + (i - 1);

				sum[0] += normals[index].X;
				sum[1] += normals[index].Y;
				sum[2] += normals[index].Z;
			}

			// Upper right face.
			if ((i < (_terrainWidth - 1)) && (j < (_terrainHeight - 1)))
			{
				index = (j * (_terrainWidth - 1)) + i;

				sum[0] += normals[index].X;
				sum[1] += normals[index].Y;
				sum[2] += normals[index].Z;
			}

			// Calculate the length of this normal.
			length = (float)sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * _terrainWidth) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			_heightMap[index].NX = (sum[0] / length);
			_heightMap[index].NY = (sum[1] / length);
			_heightMap[index].NZ = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;

	return true;
}

bool Terrain::BuildTerrainModel()
{
	int i, j, index, index1, index2, index3, index4;

	// Calculate the number of vertices in the 3D terrain model.
	_vertexCount = (_terrainHeight - 1) * (_terrainWidth - 1) * 6;

	// Create the 3D terrain model array.
	_terrainModel = new ModelType[_vertexCount];
	if (!_terrainModel)
	{
		return false;
	}

	// Initialize the index into the height map array.
	index = 0;

	// Load the 3D terrain model with the height map terrain data.
	// We will be creating 2 triangles for each of the four points in a quad.
	for (j = 0; j<(_terrainHeight - 1); j++)
	{
		for (i = 0; i<(_terrainWidth - 1); i++)
		{
			// Get the indexes to the four points of the quad.
			index1 = (_terrainWidth * j) + i;          // Upper left.
			index2 = (_terrainWidth * j) + (i + 1);      // Upper right.
			index3 = (_terrainWidth * (j + 1)) + i;      // Bottom left.
			index4 = (_terrainWidth * (j + 1)) + (i + 1);  // Bottom right.

			// Now create two triangles for that quad.
			// Triangle 1 - Upper left.
			_terrainModel[index].X = _heightMap[index1].X;
			_terrainModel[index].Y = _heightMap[index1].Y;
			_terrainModel[index].Z = _heightMap[index1].Z;
			_terrainModel[index].Tu = 0.0f;
			_terrainModel[index].Tv = 0.0f;
			_terrainModel[index].NX = _heightMap[index1].NX;
			_terrainModel[index].NY = _heightMap[index1].NY;
			_terrainModel[index].NZ = _heightMap[index1].NZ;
			index++;

			// Triangle 1 - Upper right.
			_terrainModel[index].X = _heightMap[index2].X;
			_terrainModel[index].Y = _heightMap[index2].Y;
			_terrainModel[index].Z = _heightMap[index2].Z;
			_terrainModel[index].Tu = 1.0f;
			_terrainModel[index].Tv = 0.0f;
			_terrainModel[index].NX = _heightMap[index2].NX;
			_terrainModel[index].NY = _heightMap[index2].NY;
			_terrainModel[index].NZ = _heightMap[index2].NZ;
			index++;

			// Triangle 1 - Bottom left.
			_terrainModel[index].X = _heightMap[index3].X;
			_terrainModel[index].Y = _heightMap[index3].Y;
			_terrainModel[index].Z = _heightMap[index3].Z;
			_terrainModel[index].Tu = 0.0f;
			_terrainModel[index].Tv = 1.0f;
			_terrainModel[index].NX = _heightMap[index3].NX;
			_terrainModel[index].NY = _heightMap[index3].NY;
			_terrainModel[index].NZ = _heightMap[index3].NZ;
			index++;

			// Triangle 2 - Bottom left.
			_terrainModel[index].X = _heightMap[index3].X;
			_terrainModel[index].Y = _heightMap[index3].Y;
			_terrainModel[index].Z = _heightMap[index3].Z;
			_terrainModel[index].Tu = 0.0f;
			_terrainModel[index].Tv = 1.0f;
			_terrainModel[index].NX = _heightMap[index3].NX;
			_terrainModel[index].NY = _heightMap[index3].NY;
			_terrainModel[index].NZ = _heightMap[index3].NZ;
			index++;

			// Triangle 2 - Upper right.
			_terrainModel[index].X = _heightMap[index2].X;
			_terrainModel[index].Y = _heightMap[index2].Y;
			_terrainModel[index].Z = _heightMap[index2].Z;
			_terrainModel[index].Tu = 1.0f;
			_terrainModel[index].Tv = 0.0f;
			_terrainModel[index].NX = _heightMap[index2].NX;
			_terrainModel[index].NY = _heightMap[index2].NY;
			_terrainModel[index].NZ = _heightMap[index2].NZ;
			index++;

			// Triangle 2 - Bottom right.
			_terrainModel[index].X = _heightMap[index4].X;
			_terrainModel[index].Y = _heightMap[index4].Y;
			_terrainModel[index].Z = _heightMap[index4].Z;
			_terrainModel[index].Tu = 1.0f;
			_terrainModel[index].Tv = 1.0f;
			_terrainModel[index].NX = _heightMap[index4].NX;
			_terrainModel[index].NY = _heightMap[index4].NY;
			_terrainModel[index].NZ = _heightMap[index4].NZ;
			index++;
		}
	}

	return true;
}

void Terrain::DestroyTerrainModel()
{
	// Release the terrain model data.
	if (_terrainModel)
	{
		delete[] _terrainModel;
		_terrainModel = 0;
	}

	return;
}

bool Terrain::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i, j, terrainWidth, terrainHeight;
	XMFLOAT4 color;
	float positionX, positionZ;

	// Set the height and width of the terrain grid.
	terrainHeight = 256;
	terrainWidth = 256;

	// Set the color of the terrain grid.
	color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	
	// Calculate the number of vertices in the terrain.
	_vertexCount = (_terrainWidth - 1) * (_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	_indexCount = _vertexCount;

	// Create the vertex array.
	vertices = new VertexType[_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[_indexCount];
	if(!indices)
	{
		return false;
	}

	// Load the vertex array and index array with 3D terrain model data.
	for (i = 0; i<_vertexCount; i++)
	{
		vertices[i].Position = XMFLOAT3(_terrainModel[i].X, _terrainModel[i].Y, _terrainModel[i].Z);
		vertices[i].Texture = XMFLOAT2(_terrainModel[i].Tu, _terrainModel[i].Tv);
		vertices[i].Normal = XMFLOAT3(_terrainModel[i].NX, _terrainModel[i].NY, _terrainModel[i].NZ);
		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * _vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * _indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void Terrain::DestroyBuffers()
{
	// Release the index buffer.
	if(_indexBuffer)
	{
		_indexBuffer->Release();
		_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(_vertexBuffer)
	{
		_vertexBuffer->Release();
		_vertexBuffer = 0;
	}

	return;
}

void Terrain::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}