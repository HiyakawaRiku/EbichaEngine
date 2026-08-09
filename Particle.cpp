#include "Particle.h"
#include <numbers>

std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());

ParticleData MakeNewParticle(std::mt19937& randomEngine,const Vector3& translate) {
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
		particles.push_back(MakeNewParticle(randomEngine,emitter.transform.translate));
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

	particles_.resize(kNumMaxInstance);
	for (size_t i = 0; i < kNumMaxInstance; ++i) {
		particles_.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
	}

	emitter.transform={ {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f,1.0f} };
	emitter.count = 100;
	emitter.frequency = 0.5f;
	emitter.frequencyTime = 0.0f;

}

void Particle::Update(Camera* activeCamera)
{
	// カメラ参照を保持
	activeCamera_ = activeCamera;

	emitter.frequencyTime += kDeltaTime;
	if (emitter.frequency <= emitter.frequencyTime) {
	particles_.splice(particles_.end(), Emit(emitter, randomEngine));
	emitter.frequencyTime -= emitter.frequency;
	}

	ImGui::Begin("Particle");
	ImGui::DragFloat3("EmitterTranslate", &emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::End();
}

void Particle::Draw()
{
	// 2. 1回の描画呼び出しで一括描画！
	model_->DrawInstanced(particles_, activeCamera_, textureHandle_);
}
