#include "GameOverScene.h"

void GameOverScene::Initialize()
{
    finished_ = false;

    model_ = std::make_unique<GameOverLogo>();
    model_->Initialize();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();

    // スプライトとテクスチャの初期化[cite: 5]
    uiTexture_ = TextureManager::GetInstance()->Load("resources/ground_snow.png", DirectXCommon::GetInstance()->GetCommandList());

    uiSprite_ = std::make_unique<Sprite>();
    uiSprite_->Initialize();
    uiSprite_->size = { 1280.0f, 720.0f };
    uiSprite_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void GameOverScene::Update()
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

    // スプライトの更新[cite: 5]
    if (uiSprite_) {
        uiSprite_->Update();
    }
}

void GameOverScene::Draw()
{
    // スプライトの描画[cite: 5]
    if (uiSprite_) {
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kDisable);
        uiSprite_->Draw(uiTexture_);
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kAdd, DepthWrite::kEnable);
    }

    if (model_) {
        model_->Draw();
    }
}

void GameOverScene::Finalize()
{
}