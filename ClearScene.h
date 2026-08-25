#pragma once
#include <memory>
#include "EbichaEngine.h"
#include "FadeManager.h"

class ClearScene
{
public:
	ClearScene() = default;
	~ClearScene() = default;

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