#pragma once

#include "TargaTexture.h"
#include "DDSTexture.h"

#include <map>

class TextureManager
{
public:
	TextureManager(); 
	~TextureManager();

	bool Initialize(int targaCount, int jpgCount);
	void Destroy();

	bool LoadTargaTexture(ID3D11Device*, ID3D11DeviceContext*, char*, int);
	bool LoadJPEGTexture(ID3D11Device*, ID3D11DeviceContext*, std::wstring, int);

	ID3D11ShaderResourceView* GetTexture(int);

private:
	std::map<int,ITexture*>		_textureArray;
	int							_textureCount;
};