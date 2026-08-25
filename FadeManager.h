#pragma once
#include "EbichaEngine.h"

class FadeManager {
public:
    enum class State {
        None,       // 停止中
        FadeOut,    // 明 -> 暗（シーン終了時）
        FadeIn,     // 暗 -> 明（シーン開始時）
    };

    static FadeManager* GetInstance();

    void Initialize();
    void Update();
    void Draw();

    // フェード開始命令（時間指定：秒）
    void StartFadeOut(float duration = 0.5f);
    void StartFadeIn(float duration = 0.5f);

    // 状態判定用
    bool IsFading() const { return state_ != State::None; }
    bool IsFadeOutFinished() const { return state_ == State::None && alpha_ >= 1.0f; }
    bool IsFadeInFinished() const { return state_ == State::None && alpha_ <= 0.0f; }

private:
    FadeManager() = default;
    ~FadeManager() = default;

    State state_ = State::None;
    float alpha_ = 0.0f;
    float duration_ = 0.5f;
    float timer_ = 0.0f;

    std::unique_ptr<Sprite> sprite_ = nullptr;
    TextureHandle whiteTexture_; // 白または黒のテクスチャハンドル
};