#include "HatSphere.h"
#include "Player.h"
#include <cmath>
#include <algorithm>

void HatSphere::Initialize(const Vector3& initialPos)
{
    std::vector<BaseCharacter::PartConfig> partConfigs = {};
    BaseCharacter::Initialize("sphere", partConfigs, "resources/white1x1.png");

    transformBase_.translate = initialPos;
    radius_ = 0.5f;
    SetColliderRadius(radius_);

    state_ = State::OnGround;
    ownerPlayer_ = nullptr;
    velocity_ = { 0.0f, 0.0f, 0.0f };
    isDead_ = false;
    disappearTimer_ = 0.0f;

    if (modelBody_) {
        modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };
    }
}

// ★ 1. プレイヤー装備処理の実装
void HatSphere::EquipToPlayer(Player* player)
{
    state_ = State::Equipped;
    ownerPlayer_ = player;
}

// ★ 2. 投擲・射出処理の実装
void HatSphere::Throw(const Vector3& velocity)
{
    velocity_ = velocity;
    state_ = State::Thrown;
    ownerPlayer_ = nullptr;
}

// ★ 3. 衝突時処理の実装
void HatSphere::OnHit()
{
    velocity_.x = -velocity_.x * 0.3f;
    velocity_.z = -velocity_.z * 0.3f;
    velocity_.y = 0.15f;
    state_ = State::Disappearing;
}

void HatSphere::Update(Camera* activeCamera)
{
    if (state_ == State::Equipped && ownerPlayer_) {
        // 装備中: プレイヤーの頭上に追従
        Vector3 playerPos = ownerPlayer_->GetTransform().translate;
        transformBase_.translate = {
            playerPos.x,
            playerPos.y + headOffset_,
            playerPos.z
        };
    }
    else if (state_ == State::Thrown) {
        // 重力適用と移動
        velocity_.y -= gravity_;
        transformBase_.translate += velocity_;

        // 地面着地判定
        if (transformBase_.translate.y <= groundY_) {
            transformBase_.translate.y = groundY_;

            if (std::abs(velocity_.y) > 0.08f) {
                velocity_.y = -velocity_.y * bounceFriction_;
                velocity_.x *= bounceFriction_;
                velocity_.z *= bounceFriction_;
            }
            else {
                // 着地停止後は消滅イージングへ
                state_ = State::Disappearing;
                velocity_.y = 0.0f;
            }
        }
    }
    else if (state_ == State::Disappearing) {
        // スーッと滑りながら慣性減衰
        transformBase_.translate += velocity_;
        velocity_.x *= 0.85f;
        velocity_.z *= 0.85f;

        // イージング縮小処理 (Ease-Out)
        disappearTimer_ += 1.0f / 60.0f;
        float t = std::min(disappearTimer_ / kDisappearTime_, 1.0f);
        float scale = (1.0f - t) * (1.0f - t);

        if (modelBody_) {
            modelBody_->transform.scale = { scale, scale, scale };
        }

        if (t >= 1.0f) {
            isDead_ = true;
        }
    }

    BaseCharacter::Update(activeCamera);
}