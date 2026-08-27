#include "GameOverScene.h"

void GameOverScene::Initialize()
{
    finished_ = false;

    model_ = std::make_unique<GameOverLogo>();
    model_->Initialize();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();

    uiTexture_ = TextureManager::GetInstance()->Load("resources/ground_snow.png", DirectXCommon::GetInstance()->GetCommandList());

    uiSprite_ = std::make_unique<Sprite>();
    uiSprite_->Initialize();
    uiSprite_->size = { 1280.0f, 720.0f };
    uiSprite_->color = { 1.0f, 1.0f, 1.0f, 1.0f };

    // 音声ファイルのロード & BGM再生開始 (ファイルパスは適宜調整してください)[cite: 7]
    bgmHandle_ = Audio::GetInstance()->LoadAudioSource("Resources/gameover.mp3");
    Audio::GetInstance()->PlayWave(bgmHandle_, false, 0.5f);
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

    if (uiSprite_) {
        uiSprite_->Update();
    }
}

void GameOverScene::Draw()
{
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
    // 音声リソースの解放[cite: 7]
    if (Audio::GetInstance()) {
        Audio::GetInstance()->Unload(bgmHandle_);
    }
}