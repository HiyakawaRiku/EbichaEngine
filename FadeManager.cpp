#include "FadeManager.h"

FadeManager* FadeManager::GetInstance() {
    static FadeManager instance;
    return &instance;
}

void FadeManager::Initialize() {
    state_ = State::None;
    alpha_ = 0.0f;
    timer_ = 0.0f;

    whiteTexture_ = TextureManager::GetInstance()->Load("resources/white1280x720.png", DirectXCommon::GetInstance()->GetCommandList());

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize();
    sprite_->size = { 1280,720 };
    sprite_->color = { 0.0f,0.0f,0.0f,alpha_ };

}

void FadeManager::StartFadeOut(float duration) {
    state_ = State::FadeOut;
    duration_ = (duration > 0.0f) ? duration : 0.01f;
    timer_ = 0.0f;
    alpha_ = 0.0f;
}

void FadeManager::StartFadeIn(float duration) {
    state_ = State::FadeIn;
    duration_ = (duration > 0.0f) ? duration : 0.01f;
    timer_ = 0.0f;
    alpha_ = 1.0f;
}

void FadeManager::Update() {
    if (state_ == State::None) return;


    // 60FPS想定のデルタタイム（DeltaTime管理クラスがあればそちらに差し替え）
    constexpr float kDeltaTime = 1.0f / 60.0f;
    timer_ += kDeltaTime;

    float rate = timer_ / duration_;
    if (rate > 1.0f) rate = 1.0f;

    if (state_ == State::FadeOut) {
        alpha_ = rate; // 0.0 -> 1.0
        if (rate >= 1.0f) {
            state_ = State::None;
        }
    }
    else if (state_ == State::FadeIn) {
        alpha_ = 1.0f - rate; // 1.0 -> 0.0
        if (rate >= 1.0f) {
            state_ = State::None;
        }
    }
    sprite_->Update();
}

void FadeManager::Draw() {
    if (alpha_ <= 0.0f) return;

    sprite_->color = { 0.0f,0.0f,0.0f,alpha_ };
    DirectXCommon::GetInstance()->SetPipeline(PipelineType::kParticle, BlendMode::kNormal, DepthWrite::kDisable);
    sprite_->Draw(whiteTexture_);
    // =========================================================
    // 先ほど確認できたフェード用の描画処理をここに記述
    // （例: alpha_ の値をアルファ値として適用した黒スプライトの描画）
    // =========================================================
}