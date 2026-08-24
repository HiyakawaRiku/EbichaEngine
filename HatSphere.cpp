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

void HatSphere::EquipToPlayer(Player* player)
{
    state_ = State::Equipped;
    ownerPlayer_ = player;
    thrownByPlayer_ = false;    // ★ リセット
    thrownGroundTimer_ = 0.0f;

    if (modelBody_) {
        modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };
    }
}

// HatSphere.cpp
void HatSphere::Throw(const Vector3& velocity, bool isPlayer)
{
    velocity_ = velocity;
    state_ = State::Thrown;
    ownerPlayer_ = nullptr;
    thrownByPlayer_ = isPlayer;
    isThrownByPlayer_ = isPlayer; // ★ ここを追加！（フラグを更新）
    thrownGroundTimer_ = 0.0f;

    isDangerous_ = !isPlayer;
}

// ★ 衝突時の挙動（ヒットしたらバウンドして落ちる）
void HatSphere::OnHit()
{
    velocity_.x = -velocity_.x * 0.3f;
    velocity_.z = -velocity_.z * 0.3f;
    velocity_.y = 0.15f;
    state_ = State::Thrown; // ★ 消滅せず落下運動へ移行させる
}

void HatSphere::OnHitPlayer(const Vector3& bounceVelocity)
{
    velocity_ = bounceVelocity;
    state_ = State::Thrown; // 飛翔状態に戻して反発移動させる
}

void HatSphere::Update(Camera* activeCamera)
{
    if (state_ == State::Equipped && ownerPlayer_) {
        Vector3 playerPos = ownerPlayer_->GetTransform().translate;
        transformBase_.translate = { playerPos.x, playerPos.y + headOffset_, playerPos.z };
        isDangerous_ = false;
        dangerousTimer_ = 0.0f;
    }
    else if (state_ == State::Thrown) {
        velocity_.y -= gravity_;
        transformBase_.translate += velocity_;

        float targetY = groundY_ + radius_;
        if (transformBase_.translate.y <= targetY) {
            transformBase_.translate.y = targetY;

            if (std::abs(velocity_.y) > 0.08f) {
                velocity_.y = -velocity_.y * bounceFriction_;
                velocity_.x *= bounceFriction_;
                velocity_.z *= bounceFriction_;
            }
            else {
                state_ = State::OnGround;
                velocity_ = { 0.0f, 0.0f, 0.0f };
            }
        }
    }
    else if (state_ == State::OnGround) {
        velocity_ = { 0.0f, 0.0f, 0.0f };

        // ★ 着地後も危険状態タイマーをカウント
        if (isDangerous_) {
            dangerousTimer_ += 1.0f / 60.0f;
            if (dangerousTimer_ >= kMaxDangerousTime_) {
                isDangerous_ = false;
                dangerousTimer_ = 0.0f;
            }
        }

        if (thrownByPlayer_) {
            thrownGroundTimer_ += 1.0f / 60.0f;
            if (thrownGroundTimer_ >= kMaxThrownGroundTime_) {
                state_ = State::Disappearing;
                disappearTimer_ = 0.0f;
            }
        }
    }
    else if (state_ == State::Disappearing) {
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

    // ★ 危険状態に応じた色の適用
    if (modelBody_) {
        if (isDangerous_) {
            modelBody_->color = { 1.0f, 0.2f, 0.2f, 1.0f }; // 赤色
        }
        else {
            modelBody_->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 通常色
        }
    }

    BaseCharacter::Update(activeCamera);
}