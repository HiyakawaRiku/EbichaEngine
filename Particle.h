#pragma once
#include "EbichaEngine.h"
#include "Struct.h"

struct Emitter {
	Transform transform;
	uint32_t count;
	float frequency;//発生頻度
	float frequencyTime;//頻度用時刻
};

class Particle {
public:
	~Particle(); // デストラクタを追加（delete忘れ防止）

	void Initialize();
	void Update(Camera* activeCamera_);
	void Draw();

	Emitter emitter{};
private:
	Model* model_ = nullptr;
	TextureHandle textureHandle_ = TextureManager::kInvalidHandle;
	Camera* activeCamera_ = nullptr; // カメラ参照を保持用に追加

	std::list<ParticleData> particles_;
	const uint32_t kNumMaxInstance = 10;

};

