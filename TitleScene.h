#pragma once
#include <memory>
#include "EbichaEngine.h"
#include "TitleLogo.h"
#include "FadeManager.h"

class TitleScene
{
public:
	TitleScene() = default;
	~TitleScene() = default;

	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	// getter
	bool IsFinished() const { return finished_; }

private:
	bool finished_ = false;


	std::unique_ptr<Camera> activeCamera_ = nullptr;
	std::unique_ptr<TitleLogo> model_ = nullptr;
	float testAlpha_ = 1.0f;
	BlendMode blendMode_ = BlendMode::kAdd;
};