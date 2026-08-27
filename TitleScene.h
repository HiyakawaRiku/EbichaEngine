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
	BlendMode blendMode_ = BlendMode::kAdd;

	float testAlpha_ = 1.0f;
	std::unique_ptr<TitleLogo> model_ = nullptr;

	std::unique_ptr<Sprite> uiSprite_ = nullptr;
	TextureHandle uiTexture_ = 0;
};