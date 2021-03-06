#pragma once

#include "IScene.h"

#include "OrthoWindow.h"
#include "RenderTextureBuffer.h"

class SceneDeferredShading : public IScene
{
public:
	SceneDeferredShading();

	bool Initialize(DX11Instance* Direct3D, HWND hwnd, int screenWidth, int screenHeight, float screenDepth) override;
	void Destroy() override;
	bool Update(DX11Instance* direct3D, Input* input, ShaderManager* shaderManager, float frameTime) override;

private:
	void ProcessInput(Input*, float) override;
	bool Draw(DX11Instance*, ShaderManager*) override;
	bool RenderSceneToTexture(DX11Instance* direct3D, ShaderManager* shaderManager);

	Light*					_light;
	Object*					_cube;
	OrthoWindow*			_window;
	RenderTextureBuffer*	_renderTextureBuffer;
};

