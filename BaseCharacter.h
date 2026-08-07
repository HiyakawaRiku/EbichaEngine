#pragma once
#include "EbichaEngine.h"

class BaseCharacter
{
public:
    struct PartConfig {
        std::string filename;
        Vector3 position;
    };

    virtual ~BaseCharacter() = default;

    // 基本設定を受け取る初期化関数
    virtual void Initialize(const std::string& bodyFilename, const std::vector<PartConfig>& partConfigs, const std::string& texturePath);
    virtual void Update(Camera* activeCamera);
    virtual void Draw();

    // ゲッター・セッター
    void SetViewProjection(Camera* viewProjection) { viewProjection_ = viewProjection; }
    const Transform& GetTransform() const { return transformBase_; }

protected:
    Transform transformBase_;
    std::unique_ptr<Model> modelBody_;
    std::vector<std::unique_ptr<Model>> modelParts_;
    TextureHandle textureHandle_ = TextureManager::kInvalidHandle;
    Camera* viewProjection_ = nullptr;
};

