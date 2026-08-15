#include "EnemyBullet.h"

void EnemyBullet::Initialize(const Vector3& position, const Vector3& velocity) {
    std::vector<BaseCharacter::PartConfig> partConfigs = {};
    // 弾用のモデル・テクスチャを指定（適宜変更してください）
    BaseCharacter::Initialize("enemy", partConfigs, "resources/white1x1.png");

    transformBase_.translate = position;
    transformBase_.scale = { 0.3f, 0.3f, 0.3f }; // 弾なので小さく設定
    velocity_ = velocity;

    SetColliderRadius(0.3f);
}

void EnemyBullet::Update(Camera* activeCamera) {
    transformBase_.translate += velocity_;

    deathTimer_ += 1.0f;
    if (deathTimer_ >= kLifeTime) {
        isDead_ = true;
    }

    BaseCharacter::Update(activeCamera);
}