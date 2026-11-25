#pragma once
#include <memory>
#include <cstdint>
#include "BaseScene.h"
#include "Sprite.h"

// ゲームオーバー画面
class GameOverScene : public BaseScene {
public:
    void Initialize() override;
    void Update(Input* input) override;
    void Draw() override;
    void Finalize() override;

private:
    std::unique_ptr<Sprite> sprite_;
};