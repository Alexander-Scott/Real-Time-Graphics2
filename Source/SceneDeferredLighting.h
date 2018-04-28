#pragma once

#include "IScene.h"

#include "Cube.h"
#include "OrthoWindow.h"
#include "RenderTextureBuffer.h"

class SceneDeferredLighting : public IScene
{
public:
	SceneDeferredLighting();

	bool Initialize(DX11Instance* Direct3D, HWND hwnd, int screenWidth, int screenHeight, float screenDepth) override;
	void Destroy() override;
	bool Update(DX11Instance* direct3D, Input* input, ShaderManager* shaderManager, float frameTime) override;

private:
	void ProcessInput(Input*, float) override;
	bool Draw(DX11Instance*, ShaderManager*, TextureManager*) override;
	bool RenderSceneToTexture(DX11Instance* direct3D, ShaderManager* shaderManager, TextureManager* textureManager);

	Light*					_light;
	Cube*					_cube;
	OrthoWindow*			_window;
	RenderTextureBuffer*	_renderTextureBuffer;
};

