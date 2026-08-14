#pragma once
#include "EbichaEngine.h"

class Enemy
{
public:
	~Enemy();

	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

	const Transform& GetTransform() const { return transformBase_; }

private:
	Transform transformBase_;
	Model* model_ = nullptr; // 単一のモデル
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;
	Camera* activeCamera_ = nullptr;

	// --- アニメーション用パラメータ ---
	float idleTimer_ = 0.0f;
	static inline const float kIdleSpeed = 0.05f;     // 息づかいの速度
	static inline const float kIdleBreathing = 0.2f;  // 上下揺れの大きさ
	static inline const float kIdleSquash = 0.05f;    // 息づかいでの体の伸縮量

	float walkTimer_ = 0.0f;
	static inline const float kWalkSpeed = 0.15f;    // 歩行（ホップ）速度
	static inline const float kWalkHopHeight = 0.3f; // 跳ねる高さ
	static inline const float kWalkTilt = 0.1f;      // 前後への傾き
};