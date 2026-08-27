#pragma once
#include <memory>
#include "EbichaEngine.h"
#include "FadeManager.h"
#include "gameOverLogo.h"
#include "Sprite.h" // Spriteクラスのヘッダをインクルード

class GameOverScene
{
public:
	GameOverScene() = default;
	~GameOverScene() = default;

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	bool IsFinished() const { return finished_; }

private:
	bool finished_ = false;

	std::unique_ptr<Camera> camera_ = nullptr;

	float testAlpha_ = 1.0f;
	std::unique_ptr<GameOverLogo> model_ = nullptr;

	// スプライト関連メンバ変数の追加[cite: 6]
	std::unique_ptr<Sprite> uiSprite_ = nullptr;
	TextureHandle uiTexture_ = 0;
};