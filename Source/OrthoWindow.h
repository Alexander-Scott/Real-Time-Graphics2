#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class OrthoWindow
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	OrthoWindow();
	OrthoWindow(const OrthoWindow&);
	~OrthoWindow();

	bool Initialize(ID3D11Device*, int, int);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*, int, int);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer * _vertexBuffer, *_indexBuffer;
	int _vertexCount, _indexCount;
};