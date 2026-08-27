#pragma once
#include <memory>
#include "EbichaEngine.h"
#include "FadeManager.h"
#include "clearLogo.h"
#include "Particle.h" // 追加[cite: 1, 10]

class ClearScene
{
public:
	ClearScene() = default;
	~ClearScene() = default;

	void Initialize();
	void Update();
	void Draw();

	bool IsFinished() const { return finished_; }

private:
	bool finished_ = false;

	std::unique_ptr<Camera> camera_ = nullptr;
	float testAlpha_ = 1.0f;
	std::unique_ptr<ClearLogo> model_ = nullptr;

	// モデル不要の標準パーティクル[cite: 1, 10]
	std::unique_ptr<Particle> particle_ = nullptr;
};