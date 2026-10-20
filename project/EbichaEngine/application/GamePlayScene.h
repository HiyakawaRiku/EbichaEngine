#pragma once

#include <memory>
#include <cstdint>

#include "BaseScene.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ModelCommon.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Camera.h"
#include "Sound.h"
#include "ParticleEmitter.h"

class GamePlayScene : public BaseScene {
public:
    GamePlayScene() = default;
    ~GamePlayScene() override;

    void Initialize() override;
    void Update(Input* input) override;
    void Draw() override;
    void Finalize() override;

private:
    // シーン所有のリソース・オブジェクト
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Sprite> sprite_;
    std::unique_ptr<Sprite> sprite2_;
    std::unique_ptr<Model> model_;
    std::unique_ptr<Object3d> object3d_;
    std::unique_ptr<Object3d> object3d2_;
    std::unique_ptr<ParticleEmitter> emitter_;

    uint32_t textureIndex_ = 0;
    uint32_t textureIndex2_ = 0;
    bool useMonsterBall_ = true;
    SoundData soundData1_{};
    IXAudio2SourceVoice* pSourceVoice_ = nullptr;
};