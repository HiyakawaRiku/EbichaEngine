#include "Particle.h"
#include <numbers>

std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());

ParticleData MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
	ParticleData particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	Vector3 randomTranslate{ distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.transform.translate = translate + randomTranslate;
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine),1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
}

std::list<ParticleData> Emit(const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<ParticleData> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
	}
	return particles;
}

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

Particle::~Particle()
{
	delete model_;
	model_ = nullptr;
}

void Particle::Initialize()
{
	model_ = new Model();
	model_->Initialize("plane.obj");
	textureHandle_ = TextureManager::GetInstance()->Load("resources/circle.png", DirectXCommon::GetInstance()->GetCommandList());

	emitter.transform = { {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	emitter.count = 10;
	emitter.frequency = 0.1f;
	emitter.frequencyTime = 0.0f;
	emitter.velocity = { 1.0f,1.0f,1.0f };

	particles_.clear();
	for (uint32_t i = 0; i < emitter.count; ++i) {
		particles_.push_back(MakeNewParticle(randomEngine, emitter.transform.translate)); 
	}

	accelerationField.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField.area.min = { -10.0f,-10.0f,-10.0f };
	accelerationField.area.max = { 10.0f,10.0f,10.0f };

}

void Particle::Update(Camera* activeCamera)
{
	// カメラ参照を保持
	activeCamera_ = activeCamera;
	model_->Update(activeCamera);

	// エミッターのタイマー更新と発生処理
	emitter.frequencyTime += kDeltaTime;
	if (emitter.frequency <= emitter.frequencyTime) {
		particles_.splice(particles_.end(), Emit(emitter, randomEngine));
		emitter.frequencyTime -= emitter.frequency;
	}

	// 資料通りの iterator を使用した for ループ処理[cite: 5]
	for (std::list<ParticleData>::iterator particleIterator = particles_.begin();
		particleIterator != particles_.end(); ) { 

		// 寿命が切れたパーティクルを削除（erase）する場合のパターン
		if ((*particleIterator).currentTime >= (*particleIterator).lifeTime) {
			particleIterator = particles_.erase(particleIterator); // 削除して次のイテレータを取得
			continue;
		}

		if (isUpdate) {

			if (Physics3D::IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
				(*particleIterator).velocity += accelerationField.acceleration * kDeltaTime;
			}
		}

		// 移動・経過時間の更新処理
		(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;
		(*particleIterator).currentTime += kDeltaTime;
		++particleIterator;

	}

	ImGui::Begin("Particle");
	ImGui::DragFloat3("EmitterTranslate", &emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat3("EmitterTranslat", &accelerationField.acceleration.x, 0.01f, -100.0f, 100.0f);
	ImGui::Checkbox("update", &isUpdate);
	ImGui::End();
}

void Particle::Draw()
{
	// 2. 1回の描画呼び出しで一括描画！
	model_->DrawInstanced(particles_, activeCamera_, textureHandle_);
}
