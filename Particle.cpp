#include "Particle.h"
#include <numbers>

std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());

ParticleData MakeNewParticle(std::mt19937& randomEngine) {
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
	ParticleData particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine),1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
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
	textureHandle_ = TextureManager::GetInstance()->Load("resources/ground_leaf.png", DirectXCommon::GetInstance()->GetCommandList());

	particles_.resize(kNumMaxInstance);
	for (size_t i = 0; i < particles_.size(); ++i) {
		particles_[i] = MakeNewParticle(randomEngine);
	}


}

void Particle::Update(Camera* activeCamera)
{
	// カメラ参照を保持
	activeCamera_ = activeCamera;

	for (auto& particle : particles_) {
		particle.transform.translate += particle.velocity * kDeltaTime;
	}
}

void Particle::Draw()
{
	// 2. 1回の描画呼び出しで一括描画！
	model_->DrawInstanced(particles_, activeCamera_, textureHandle_);
}
