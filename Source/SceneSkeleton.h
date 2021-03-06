#pragma once

#include "IScene.h"

class SceneSkeleton : public IScene
{
public:
	SceneSkeleton();
	~SceneSkeleton();

	bool Initialize(DX11Instance* Direct3D, HWND hwnd, int screenWidth, int screenHeight, float screenDepth) override;
	void Destroy() override;
	bool Update(DX11Instance* direct3D, Input* input, ShaderManager* shaderManager, float frameTime) override;

private:
	void ProcessInput(Input*, float) override;
	bool Draw(DX11Instance*, ShaderManager*) override;

	Skeleton* _skeleton;
};

