#include "BaseCharacter.h"

void BaseCharacter::Initialize(const std::string& bodyFilename, const std::vector<PartConfig>& partConfigs, const std::string& texturePath)
{
    transformBase_.Initialize(); //[cite: 5]

    // 胴体モデル生成[cite: 5]
    modelBody_ = std::make_unique<Model>();
    modelBody_->Initialize(bodyFilename); //[cite: 5]
    modelBody_->transform.translate = { 0.0f, 0.0f, 0.0f }; //[cite: 5]
    modelBody_->transform.parent = &transformBase_; //[cite: 5]

    // パーツモデル生成[cite: 5]
    modelParts_.clear(); //[cite: 5]
    for (const auto& config : partConfigs) {
        auto part = std::make_unique<Model>();
        part->Initialize(config.filename); //[cite: 5]
        part->transform.translate = config.position; //[cite: 5]
        part->transform.parent = &modelBody_->transform; //[cite: 5]
        modelParts_.push_back(std::move(part)); //[cite: 5]
    }

    // テクスチャロード[cite: 5]
    textureHandle_ = TextureManager::GetInstance()->Load(texturePath, DirectXCommon::GetInstance()->GetCommandList()); //[cite: 5]
}

void BaseCharacter::Update(Camera* activeCamera)
{
    viewProjection_ = activeCamera; //[cite: 5]

    // 胴体と各パーツのカメラ参照更新を一括処理
    if (modelBody_) {
        modelBody_->Update(activeCamera);
    }
    for (auto& part : modelParts_) {
        if (part) {
            part->Update(activeCamera);
        }
    }

    transformBase_.UpdateMatrix(); //[cite: 5]
}

void BaseCharacter::Draw()
{
    // 全体行列の更新[cite: 5]
    transformBase_.UpdateMatrix(); //[cite: 5]

    // 胴体の描画[cite: 5]
    if (modelBody_) {
        modelBody_->transform.UpdateMatrix(); //[cite: 5]
        modelBody_->Draw(viewProjection_, textureHandle_); //[cite: 5]
    }

    // 各パーツの描画[cite: 5]
    for (auto& part : modelParts_) {
        if (part) {
            part->transform.UpdateMatrix(); //[cite: 5]
            part->Draw(viewProjection_, textureHandle_); //[cite: 5]
        }
    }
}