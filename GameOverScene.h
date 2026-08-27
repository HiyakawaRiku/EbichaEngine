#pragma once
#include <memory>
#include "EbichaEngine.h"
#include "FadeManager.h"
#include "gameOverLogo.h"

class GameOverScene
{
public:
	GameOverScene() = default;
	~GameOverScene() = default;

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	// getter
	bool IsFinished() const { return finished_; }

private:
	bool finished_ = false;

	std::unique_ptr<Camera> camera_ = nullptr;

	float testAlpha_ = 1.0f;
	std::unique_ptr<GameOverLogo> model_ = nullptr;
};