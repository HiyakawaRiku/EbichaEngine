#pragma once
#include <memory>
#include <cstdint>
#include "BaseScene.h"
#include "Sprite.h"

// ゲームクリア画面
class ClearScene : public BaseScene {
public:
    void Initialize() override;
    void Update(Input* input) override;
    void Draw() override;
    void Finalize() override;

private:
    std::unique_ptr<Sprite> sprite_;
};