#pragma once
#include <memory>
#include "EbichaEngine.h"
#include "FadeManager.h"
#include "clearLogo.h"
#include "Particle.h"
#include "Sprite.h" // Spriteクラスのヘッダをインクルード

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

	std::unique_ptr<Particle> particle_ = nullptr;

	// スプライト関連メンバ変数の追加[cite: 6]
	std::unique_ptr<Sprite> uiSprite_ = nullptr;
	TextureHandle uiTexture_ = 0;
};