#include "ClearScene.h"
#include <random>

void ClearScene::Initialize()
{
    finished_ = false;

    model_ = std::make_unique<ClearLogo>();
    model_->Initialize();

    camera_ = std::make_unique<Camera>();
    camera_->Initialize();

    particle_ = std::make_unique<Particle>();
    particle_->Initialize();

    uiTexture_ = TextureManager::GetInstance()->Load("resources/ground_snow.png", DirectXCommon::GetInstance()->GetCommandList());

    uiSprite_ = std::make_unique<Sprite>();
    uiSprite_->Initialize();
    uiSprite_->size = { 1280.0f, 720.0f };
    uiSprite_->color = { 1.0f, 1.0f, 1.0f, 1.0f };

    // 音声ファイルのロード & BGM再生開始 (ファイルパスは適宜調整してください)[cite: 7]
    bgmHandle_ = Audio::GetInstance()->LoadAudioSource("Resources/clear.mp3");
    Audio::GetInstance()->PlayWave(bgmHandle_, false, 0.5f);
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

    if (uiSprite_) {
        uiSprite_->Update();
    }

    if (particle_) {
        static std::random_device seed;
        static std::mt19937 engine(seed());
        std::uniform_real_distribution<float> distX(-15.0f, 15.0f);
        std::uniform_real_distribution<float> distY(10.0f, 15.0f);
        std::uniform_real_distribution<float> distZ(-5.0f, 5.0f);

        for (int i = 0; i < 3; ++i) {
            Vector3 spawnPos = { distX(engine), distY(engine), distZ(engine) };
            particle_->EmitAt(spawnPos, 1);
        }

        particle_->Update(camera_.get());
    }
}

void ClearScene::Draw()
{
    if (uiSprite_) {
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kNormal, DepthWrite::kDisable);
        uiSprite_->Draw(uiTexture_);
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kObject3D, BlendMode::kAdd, DepthWrite::kEnable);
    }

    if (model_) {
        model_->Draw();
    }

    if (particle_) {
        DirectXCommon::GetInstance()->SetPipeline(PipelineType::kParticle, BlendMode::kAdd, DepthWrite::kDisable);
        particle_->Draw();
    }
}

void ClearScene::Finalize()
{
    // 音声リソースの解放[cite: 7]
    if (Audio::GetInstance()) {
        Audio::GetInstance()->Unload(bgmHandle_);
    }
}