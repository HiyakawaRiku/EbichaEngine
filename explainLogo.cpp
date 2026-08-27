#include "explainLogo.h"
#include <cmath> // std::sin 使用のため

ExplainLogo::~ExplainLogo()
{
	delete model_;
	model_ = nullptr;
}

void ExplainLogo::Initialize()
{
	model_ = new Model();
	model_->Initialize("explain");
	textureHandle_ = TextureManager::GetInstance()->Load("resources/sky_spahere.png", DirectXCommon::GetInstance()->GetCommandList());

	// 初期位置を保存（model_->transform_.translate_ などの構造に合わせて変更してください）
	initialPosition_ = model_->transform.translate;
	initialPosition_.z = -30;
}

void ExplainLogo::Update(Camera* activeCamera)
{
	activeCamera_ = activeCamera;

	// アニメーション処理（タイマー更新）
	timer_ += 0.05f; // 変化速度

	// サイン波を使って上下にゆらゆら動かす
	float amplitude = 0.5f; // 移動の幅（高さ）
	Vector3 currentPos = initialPosition_;
	currentPos.y += std::sin(timer_) * amplitude;

	// モデルの座標を更新
	model_->transform.translate = currentPos;

	model_->Update(activeCamera);
}

void ExplainLogo::Draw()
{
	if (model_) {
		model_->Draw(activeCamera_, textureHandle_);
	}
}