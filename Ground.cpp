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

void Ground::Initialize()
{
	model_ = new Model();
	model_->Initialize("plane.obj");
    textureHandle_ = TextureManager::GetInstance()->Load("resources/ground_leaf.png", DirectXCommon::GetInstance()->GetCommandList());
}

void Ground::Update(Camera* activeCamera_)
{
    float rotationDeg = 90.0f; // 度数法で指定
    float theta = MathUtils::ToRadians(rotationDeg); // ラジアン(θ)に変換
    model_->transform.rotate.x = theta;
	model_->transform.scale.x = 100;
	model_->transform.scale.y = 100;

	model_->Update(activeCamera_);
}

void Ground::Draw()
{
	model_->Draw(textureHandle_);
}
