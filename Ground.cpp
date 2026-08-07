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
	model_->Initialize("plane.obj");
	textureHandle_ = TextureManager::GetInstance()->Load("resources/ground_leaf.png", DirectXCommon::GetInstance()->GetCommandList());
}

void Ground::Update(Camera* activeCamera)
{
	// カメラ参照を保持
	activeCamera_ = activeCamera;

	float rotationDeg = 90.0f; // 度数法で指定
	float theta = MathUtils::ToRadians(rotationDeg); // ラジアン(θ)に変換

	model_->transform.rotate.x = theta;
	model_->transform.scale.x = 100.0f;
	model_->transform.scale.y = 100.0f;

	// 旧 BaseObject の Update(activeCamera_) 呼出しは不要になったため削除
}

void Ground::Draw()
{
	//// 1. 位置をずらした Transform の配列を作る
	//std::vector<Transform> transforms(10);
	//for (size_t i = 0; i < transforms.size(); ++i) {
	//	transforms[i].scale = { 1.0f, 1.0f, 1.0f };
	//	transforms[i].rotate = { 0.0f, 0.0f, 0.0f };
	//	transforms[i].translate = { i * 0.1f, i * 0.1f, i * 0.1f }; // 横に2刻みで並べる
	//}

	//// 2. 1回の描画呼び出しで一括描画！
	//model_->DrawInstanced(transforms, activeCamera_, textureHandle_);

	if (model_) {
		// 新しい Model::Draw(Camera*, TextureHandle) を呼び出す
		model_->Draw(activeCamera_, textureHandle_);
	}
}