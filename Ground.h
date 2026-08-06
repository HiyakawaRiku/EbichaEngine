#pragma once
#include "EbichaEngine.h"

class Ground
{
public:
	~Ground(); // デストラクタを追加（delete忘れ防止）

	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

private:
	Model* model_ = nullptr;
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;
	Camera* activeCamera_ = nullptr; // カメラ参照を保持用に追加
};