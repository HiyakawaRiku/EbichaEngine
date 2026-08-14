#pragma once
#include "EbichaEngine.h"
#include <memory>
#include <vector>
#include <string>

class BaseCharacter
{
public:
    struct PartConfig {
        std::string filename;
        Vector3 position;
    };

    virtual ~BaseCharacter() = default;

    // 基本設定を受け取る初期化関数[cite: 6]
    virtual void Initialize(const std::string& bodyFilename, const std::vector<PartConfig>& partConfigs, const std::string& texturePath);

    // 基本更新処理（カメラ更新と行列計算）[cite: 6]
    virtual void Update(Camera* activeCamera);

    // 共通描画処理（オーバーライド不要）[cite: 5, 6]
    virtual void Draw();

    // ゲッター・セッター[cite: 6]
    void SetViewProjection(Camera* viewProjection) { viewProjection_ = viewProjection; }
    const Transform& GetTransform() const { return transformBase_; }

protected:
    // モデル・トランスフォーム関連（親子関係）[cite: 6]
    Transform transformBase_;
    std::unique_ptr<Model> modelBody_;
    std::vector<std::unique_ptr<Model>> modelParts_;
    TextureHandle textureHandle_ = TextureManager::kInvalidHandle;
    Camera* viewProjection_ = nullptr;

    // 歩行・待機アニメーション共通パラメータ
    float walkTimer_ = 0.0f;
    static inline const float kWalkSpeed = 0.2f;
    static inline const float kWalkAngle = 0.5f;

    float idleTimer_ = 0.0f;
    static inline const float kIdleSpeed = 0.05f;
    static inline const float kIdleBreathing = 0.03f;
    static inline const float kIdleArmAngle = 0.05f;

    static inline const float kRotateSpeed = 0.15f;
};