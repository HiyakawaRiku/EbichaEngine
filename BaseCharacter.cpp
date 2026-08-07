#include "BaseCharacter.h"

void BaseCharacter::Initialize(const std::string& bodyFilename, const std::vector<PartConfig>& partConfigs, const std::string& texturePath)
{
    transformBase_.Initialize();

    // 胴体モデル生成
    modelBody_ = std::make_unique<Model>();
    modelBody_->Initialize(bodyFilename);
    modelBody_->transform.translate = { 0.0f, 0.0f, 0.0f };
    modelBody_->transform.parent = &transformBase_;

    // パーツモデル生成
    modelParts_.clear();
    for (const auto& config : partConfigs) {
        auto part = std::make_unique<Model>();
        part->Initialize(config.filename);
        part->transform.translate = config.position;
        part->transform.parent = &modelBody_->transform;
        modelParts_.push_back(std::move(part));
    }

    // テクスチャロード
    textureHandle_ = TextureManager::GetInstance()->Load(texturePath, DirectXCommon::GetInstance()->GetCommandList());
}

void BaseCharacter::Update(Camera* activeCamera)
{
    viewProjection_ = activeCamera;
    transformBase_.UpdateMatrix();
}

void BaseCharacter::Draw()
{
    transformBase_.UpdateMatrix();

    if (modelBody_) {
        modelBody_->transform.UpdateMatrix();
        modelBody_->Draw(viewProjection_, textureHandle_);
    }

    for (auto& part : modelParts_) {
        if (part) {
            part->transform.UpdateMatrix();
            part->Draw(viewProjection_, textureHandle_);
        }
    }
}