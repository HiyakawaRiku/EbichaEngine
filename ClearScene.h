#pragma once
#include <memory>
#include <cstdint> // uint32_t 用
#include "EbichaEngine.h"
#include "FadeManager.h"
#include "clearLogo.h"
#include "Particle.h"
#include "Sprite.h"

class ClearScene
{
public:
	ClearScene() = default;
	~ClearScene() = default;

	void Initialize();
	void Update();
	void Draw();
	void Finalize(); // Finalize を追加

	bool IsFinished() const { return finished_; }

private:
	bool finished_ = false;

	std::unique_ptr<Camera> camera_ = nullptr;
	float testAlpha_ = 1.0f;
	std::unique_ptr<ClearLogo> model_ = nullptr;

	std::unique_ptr<Particle> particle_ = nullptr;

	std::unique_ptr<Sprite> uiSprite_ = nullptr;
	TextureHandle uiTexture_ = 0;

	// BGM関連の変数追加[cite: 8]
	uint32_t bgmHandle_ = 0;
};