#pragma once
#include "EbichaEngine.h"

class Ground
{
public:
	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();
private:
	Model* model_ = nullptr;
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;
};

