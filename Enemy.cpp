#include "Enemy.h"
#include <cmath>

Enemy::~Enemy()
{
	delete model_;
	model_ = nullptr;
}

void Enemy::Initialize()
{
	transformBase_.Initialize();
	transformBase_.translate = { 0.0f, 5.0f, 0.0f }; // 初期位置

	model_ = new Model();
	model_->Initialize("enemy.obj");
	model_->transform.parent = &transformBase_; // トランスフォームを親子関係に

	textureHandle_ = TextureManager::GetInstance()->Load("resources/player.png", DirectXCommon::GetInstance()->GetCommandList());
}

void Enemy::Update(Camera* activeCamera)
{
	activeCamera_ = activeCamera;
	model_->Update(activeCamera);

	// ----------------------------------------------------
	// 移動判定とアニメーション
	// ----------------------------------------------------
	Vector3 moveVelocity = { 0.0f, 0.0f, 0.0f };
	// ※必要に応じて移動処理を記述（例: moveVelocity.x = 0.05f; など）

	bool isMoving = (moveVelocity.x != 0.0f || moveVelocity.y != 0.0f || moveVelocity.z != 0.0f);

	if (isMoving) {
		// --- 1. 移動中：ぴょんぴょん跳ねる・体を傾ける動き ---
		walkTimer_ += kWalkSpeed;
		float sinVal = std::sin(walkTimer_);

		// Y軸の跳ね運動
		model_->transform.translate.y = std::abs(sinVal) * kWalkHopHeight;

		// 移動方向に体を少し傾ける（臨場感）
		model_->transform.rotate.x = kWalkTilt;

		// 進行方向に向く
		float targetAngle = std::atan2(moveVelocity.x, moveVelocity.z);
		model_->transform.rotate.y = targetAngle;

		transformBase_.translate += moveVelocity;
	}
	else {
		// --- 2. 待機中：呼吸のように上下に伸縮する動き ---
		walkTimer_ = 0.0f;
		idleTimer_ += kIdleSpeed;
		float sinVal = std::sin(idleTimer_);

		// 体の位置を微小に上下
		model_->transform.translate.y = sinVal * kIdleBreathing;

		// 体を伸縮させる（息を吸うと縦伸び、吐くと横広がり）
		model_->transform.scale.x = 1.0f - (sinVal * kIdleSquash);
		model_->transform.scale.y = 1.0f + (sinVal * kIdleSquash);
		model_->transform.scale.z = 1.0f - (sinVal * kIdleSquash);

		model_->transform.rotate.x = 0.0f;
	}

	transformBase_.UpdateMatrix();
}

void Enemy::Draw()
{
	transformBase_.UpdateMatrix();
	if (model_) {
		model_->transform.UpdateMatrix();
		model_->Draw(activeCamera_, textureHandle_);
	}
}