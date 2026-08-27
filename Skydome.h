#pragma once
#include "EbichaEngine.h"

class Skydome
{
public:
	~Skydome(); // デストラクタを追加

	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

public:
	Model* model_ = nullptr;
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;

private:
	Camera* activeCamera_ = nullptr; // カメラ参照保持用
};