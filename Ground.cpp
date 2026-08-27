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
	model_->Initialize("ground");
	textureHandle_ = TextureManager::GetInstance()->Load("resources/ground_snow.png", DirectXCommon::GetInstance()->GetCommandList());


	model_->SetSpotLightDecay(0.0f);
	model_->SetSpotLightDistance(100.0f);
	model_->SetSpotLightCosAngle(std::cos(DirectX::XMConvertToRadians(0.0f)));
}

void Ground::Update(Camera* activeCamera)
{

	// カメラ参照を保持
	activeCamera_ = activeCamera;
	model_->Update(activeCamera);

	float rotationDeg = 90.0f; // 度数法で指定
	float theta = MathUtils::ToRadians(rotationDeg); // ラジアン(θ)に変換

	//model_->transform.rotate.x = theta;
	model_->transform.scale.x = 30.0f;
	model_->transform.scale.y = 30.0f;
	model_->transform.scale.z = 30.0f;
	
#ifdef _DEBUG
	ImGui::Begin("Settings");
	// =========================================================
	// ポイントライト調整用 UI
	// =========================================================
	if (PointLight* pointLight = model_->GetPointLightData()) {
		if (ImGui::TreeNode("Point Light")) {
			ImGui::ColorEdit4("Color", &pointLight->color.x);
			ImGui::DragFloat3("Position", &pointLight->position.x, 0.1f);
			ImGui::DragFloat("Intensity", &pointLight->intensity, 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("Radius", &pointLight->radius, 0.1f, 0.1f, 1000.0f);
			ImGui::DragFloat("Decay", &pointLight->decay, 0.05f, 0.0f, 10.0f);
			ImGui::TreePop();
		}
	}

	// =========================================================
	// スポットライト調整用 UI
	// =========================================================
	if (SpotLight* spotLight = model_->GetSpotLightData()) {
		if (ImGui::TreeNode("Spot Light")) {
			ImGui::ColorEdit4("Color", &spotLight->color.x);
			ImGui::DragFloat3("Position", &spotLight->position.x, 0.1f);
			ImGui::DragFloat3("Direction", &spotLight->direction.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("Intensity", &spotLight->intensity, 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("Distance", &spotLight->distance, 0.1f, 0.1f, 1000.0f);
			ImGui::DragFloat("Decay", &spotLight->decay, 0.05f, 0.0f, 10.0f);

			// 角度 (cos値から角度(度)へ変換して調整)
			static float angleDeg = 45.0f;
			static float falloffStartDeg = 30.0f;

			if (ImGui::DragFloat("Angle (deg)", &angleDeg, 0.5f, 0.0f, 90.0f)) {
				spotLight->cosAngle = std::cos(angleDeg * 3.14159265f / 180.0f);
			}
			if (ImGui::DragFloat("Falloff Start (deg)", &falloffStartDeg, 0.5f, 0.0f, angleDeg)) {
				spotLight->cosFalloffStart = std::cos(falloffStartDeg * 3.14159265f / 180.0f);
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();
#endif

}

void Ground::Draw()
{

	if (model_) {
		// 新しい Model::Draw(Camera*, TextureHandle) を呼び出す
		model_->Draw(activeCamera_, textureHandle_);
	}
}