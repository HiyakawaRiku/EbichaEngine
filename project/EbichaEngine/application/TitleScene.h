#pragma once

#include <memory>
#include <cstdint>

#include "BaseScene.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"

class TitleScene : public BaseScene {
public:
    TitleScene() = default;
    ~TitleScene() override = default;

    void Initialize() override;
    void Update(Input* input) override;
    void Draw() override;
    void Finalize() override;

private:
    uint32_t textureIndex_ = 0;
    std::unique_ptr<Sprite> titleSprite_;
};