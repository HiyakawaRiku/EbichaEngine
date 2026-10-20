#pragma once

#include <memory>
#include <cstdint>

#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ModelCommon.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Camera.h"
#include "Sound.h"
#include "ParticleEmitter.h"
#include "Input.h"

class GamePlayScene {
public:

    static GamePlayScene* GetInstance();
    GamePlayScene() = default;
    ~GamePlayScene();

    // 初期化
    void Initialize();

    // 更新
    void Update(Input* input);

    // 描画
    void Draw();

    void Finalize();

private:
    // 依存参照ポインタ（所有権はエンジン側）
    SpriteCommon* spriteCommon_ = nullptr;
    Object3dCommon* object3dCommon_ = nullptr;
    ModelCommon* modelCommon_ = nullptr;
    Sound* sound_ = nullptr;

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