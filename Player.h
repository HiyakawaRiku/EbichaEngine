#pragma once
#include "EbichaEngine.h"

class Player
{
	enum class Behavior {
		kRoot,//通常状態
		kJump,//ジャンプ中
	};

public:
	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

	void InitializeFloatingGimmick();
	void UpdateFloatingGimmick();

	void BehaviorRootInitialize();
	void BehaviorRootUpdate(Camera* activeCamera_);

	void BehaviorJumpInitialize();
	void BehaviorJumpUpdate();

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

	float walkTimer_ = 0.0f; // 歩行アニメーション用のタイマー
	static inline const float kWalkSpeed = 0.2f;    // 歩くテンポの速さ
	static inline const float kWalkAngle = 0.5f;    // 手足を振る角度（ラジアン: 約28度）

	// --- 攻撃（腕抜きブレード）アニメーション用 ---
	bool isAttacking_ = false;
	float attackTimer_ = 0.0f;
	const float kAttackDuration = 40.0f; // 攻撃の全体の長さ（30フレーム = 約0.5秒）

	// --- 待機アニメーション用 ---
	float idleTimer_ = 0.0f;
	static inline const float kIdleSpeed = 0.05f;  // 息づかいのテンポ（ゆっくり）
	static inline const float kIdleBreathing = 0.03f; // 体の上下の振幅
	static inline const float kIdleArmAngle = 0.05f;   // 腕の揺れ角度

private:
	// --- ジャンプアニメーション用 ---
	float jumpTimer_ = 0.0f;
	float jumpVelocityY_ = 0.0f;
	bool isJumping_ = false;

	static inline const float kJumpInitialVelocity = 0.4f; // 初速
	static inline const float kGravity = 0.02f;             // 重力
	static inline const float kJumpLandDuration = 10.0f;   // 着地硬直フレーム数

private:

	// ImGui調整可能パラメータ
	float jumpInitialVelocity_ = 0.4f;  // ジャンプ初速（高さ）
	float gravity_ = 0.02f;              // 重力
	float jumpSquashAmount_ = 0.3f;      // つぶれる強さ（0.0で変形なし、0.3でスケールが1.3/0.7になる）
	float jumpGroundY_ = 1.5f;           // 地面の高さ基準
};

