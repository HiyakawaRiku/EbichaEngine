#pragma once
#include "EbichaEngine.h"

class ClearLogo
{
public:
	~ClearLogo();

	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

public:
	Model* model_ = nullptr;
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;

private:
	Camera* activeCamera_ = nullptr;

	// アニメーション用の変数追加
	float timer_ = 0.0f;
	Vector3 initialPosition_ = { 0.0f, 0.0f, 0.0f }; // 初期位置
};