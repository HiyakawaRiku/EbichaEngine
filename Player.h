#pragma once
#include "EbichaEngine.h"

class Player
{
public:
	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

	void InitializeFloatingGimmick();
	void UpdateFloatingGimmick();

	void SetViewProjection(const Camera* viewProjection) { viewProjection_ = viewProjection; }
	const Transform& GetTransform()const { return transformBase_; }

private:

	std::unique_ptr<Model> modelBody_;
	std::vector<std::unique_ptr<Model>> modelParts_;

	Transform transformBase_;
	//Transform transformBody_;
	//Transform transformHead_;
	//Transform transformL_arm_;
	//Transform transformR_arm_;

	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;

	// カメラのビュープロジェクション
	const Camera* viewProjection_ = nullptr;

	static inline const float kAcceleration = 0.2f;
	static inline const float kRotateSpeed = 0.15f;

	float floatingParameter_ = 0.0f;
	float frame_ = 60.0f;
	float floatingAmplitude = 0.1f;
};

