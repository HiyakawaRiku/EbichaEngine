#pragma once
#include "EbichaEngine.h"

class Player
{
public:
	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

	void SetViewProjection(const Camera* viewProjection) { viewProjection_ = viewProjection; }
	const Transform& GetTransform()const { return modelBody_->transform; }

private:
	Model* modelBody_ = nullptr;
	Model* modelHead_ = nullptr;
	Model* modelL_arm_ = nullptr;
	Model* modelR_arm_ = nullptr;

	Transform transformBase_;
	//Transform transformBody_;
	//Transform transformHead_;
	//Transform transformL_arm_;
	//Transform transformR_arm_;

	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;

	// カメラのビュープロジェクション
	const Camera* viewProjection_ = nullptr;

	static inline const float kAcceleration = 0.2f;
};

