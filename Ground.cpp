#include "Ground.h"
#include <numbers>

namespace MathUtils {
	// deg -> rad (θ)
	constexpr float ToRadians(float degrees) {
		return degrees * (std::numbers::pi_v<float> / 180.0f);
	}

	// rad (θ) -> deg
	constexpr float ToDegrees(float radians) {
		return radians * (180.0f / std::numbers::pi_v<float>);
	}
}

Ground::~Ground()
{
	delete model_;
	model_ = nullptr;
}

void Ground::Initialize()
{
	model_ = new Model();
	model_->Initialize("cube.gltf");
	textureHandle_ = TextureManager::GetInstance()->Load("resources/grass.png", DirectXCommon::GetInstance()->GetCommandList());
}

void Ground::Update(Camera* activeCamera)
{
	// カメラ参照を保持
	activeCamera_ = activeCamera;

	//float rotationDeg = 90.0f; // 度数法で指定
	//float theta = MathUtils::ToRadians(rotationDeg); // ラジアン(θ)に変換

	//model_->transform.rotate.x = theta;
	//model_->transform.scale.x = 100.0f;
	//model_->transform.scale.y = 100.0f;

	model_->transform.translate.x = 5.0f;
}

void Ground::Draw()
{

	if (model_) {
		// 新しい Model::Draw(Camera*, TextureHandle) を呼び出す
		model_->Draw(activeCamera_, textureHandle_);
	}
}