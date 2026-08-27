#include "ClearScene.h"
#include <random> // ランダム位置生成用

void ClearScene::Initialize()
{
    finished_ = false;

    model_ = std::make_unique<ClearLogo>();
    model_->Initialize();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();

    // パーティクルの初期化[cite: 6, 9]
    particle_ = std::make_unique<Particle>();
    particle_->Initialize();
}

void ClearScene::Update()
{
    if (!FadeManager::GetInstance()->IsFading() && !finished_) {
        if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
            FadeManager::GetInstance()->StartFadeOut(0.5f);
        }
    }

    if (FadeManager::GetInstance()->IsFadeOutFinished()) {
        finished_ = true;
    }
    FadeManager::GetInstance()->Update();
    camera_->Update();

    if (model_) {
        model_->Update(camera_.get());
    }

    // パーティクルの更新と上方からの持続発生[cite: 6, 9]
    if (particle_) {
        // 1. 画面の上方（例: Y = 10〜15, X = -15〜15, Z = -5〜5）のランダムな位置を生成
        static std::random_device seed;
        static std::mt19937 engine(seed());
        std::uniform_real_distribution<float> distX(-15.0f, 15.0f);
        std::uniform_real_distribution<float> distY(10.0f, 15.0f);
        std::uniform_real_distribution<float> distZ(-5.0f, 5.0f);

        // 2. 毎フレーム数個ずつ上から発生させる[cite: 9]
        for (int i = 0; i < 3; ++i) { // 発生頻度・個数はお好みで調整
            Vector3 spawnPos = { distX(engine), distY(engine), distZ(engine) };
            particle_->EmitAt(spawnPos, 1);
        }

        particle_->Update(camera_.get());
    }
}

void ClearScene::Draw()
{
    if (model_) {
        model_->Draw();
    }

    // パーティクルの描画[cite: 9]
    if (particle_) {
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);
            particle_->Draw();
    }
}