#pragma once
#include <memory>
#include "EbichaEngine.h"

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
};