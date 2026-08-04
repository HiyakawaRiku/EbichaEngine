#pragma once
#include "EbichaEngine.h"

class Player
{
public:
	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

	void SetViewProjection(const Camera* viewProjection) { viewProjection_ = viewProjection; }
	const Transform& GetTransform()const { return model_->transform; }

private:
	Model* model_ = nullptr;
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;

	// カメラのビュープロジェクション
	const Camera* viewProjection_ = nullptr;

	static inline const float kAcceleration = 0.2f;
};

