#pragma once
#include "EbichaEngine.h"

class Skydome
{
public:
	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();
public:
	Model* model_ = nullptr;
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;

};

