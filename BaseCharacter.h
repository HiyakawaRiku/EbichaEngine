#pragma once
#include "EbichaEngine.h"
#include "Physics3D.h" // ★追加
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

    virtual void Initialize(const std::string& bodyFilename, const std::vector<PartConfig>& partConfigs, const std::string& texturePath);
    virtual void Update(Camera* activeCamera);
    virtual void Draw();

    void SetViewProjection(Camera* viewProjection) { viewProjection_ = viewProjection; }
    const Transform& GetTransform() const { return transformBase_; }

    // --- ★ 当たり判定用アクセッサを追加 ---[cite: 1]
    const BSphere& GetColliderSphere() const { return colliderSphere_; }
    void SetColliderRadius(float radius) { colliderSphere_.radius = radius; }

protected:
    Transform transformBase_;
    std::unique_ptr<Model> modelBody_;
    std::vector<std::unique_ptr<Model>> modelParts_;
    TextureHandle textureHandle_ = TextureManager::kInvalidHandle;
    Camera* viewProjection_ = nullptr;

    // --- ★ コライダー（球体）を保持 ---[cite: 1]
    BSphere colliderSphere_;

    // アニメーション用変数...
    float walkTimer_ = 0.0f;
    static inline const float kWalkSpeed = 0.2f;
    static inline const float kWalkAngle = 0.5f;

    float idleTimer_ = 0.0f;
    static inline const float kIdleSpeed = 0.05f;
    static inline const float kIdleBreathing = 0.03f;
    static inline const float kIdleArmAngle = 0.05f;

    static inline const float kRotateSpeed = 0.15f;
};