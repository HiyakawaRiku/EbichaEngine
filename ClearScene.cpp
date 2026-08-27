#include "ClearScene.h"

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

    // 例：初期化時に画面中央（原点）付近へパーティクルを発生させる
    if (particle_) {
        particle_->EmitAt({ 0.0f, 0.0f, 0.0f }, 50);
    }
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

    // パーティクルの更新[cite: 6, 9]
    if (particle_) {
        particle_->Update(camera_.get());

        // 毎フレーム継続的に散らしたい場合の例
        // particle_->EmitAt({ 0.0f, 0.0f, 0.0f }, 2);
    }
}

void ClearScene::Draw()
{
    // 3Dモデルの描画[cite: 6]
    if (model_) {
        model_->Draw();
    }

    // パーティクル専用パイプラインへ切り替えて描画[cite: 9]
    if (particle_) {
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);
            particle_->Draw();
    }
}