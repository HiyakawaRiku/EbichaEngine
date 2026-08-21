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

void HatSphere::Throw(const Vector3& velocity, bool isPlayer)
{
    velocity_ = velocity;
    state_ = State::Thrown;
    ownerPlayer_ = nullptr;
    thrownByPlayer_ = isPlayer; // ★ プレイヤーが投げた場合のみ true
    thrownGroundTimer_ = 0.0f;
}

// ★ 衝突時の挙動（ヒットしたらバウンドして落ちる）
void HatSphere::OnHit()
{
    velocity_.x = -velocity_.x * 0.3f;
    velocity_.z = -velocity_.z * 0.3f;
    velocity_.y = 0.15f;
    state_ = State::Thrown; // ★ 消滅せず落下運動へ移行させる
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

        // ★ めり込み防止（中心 Y = groundY_ + radius_）
        float targetY = groundY_ + radius_; // 0.5f + 0.5f = 1.0f
        if (transformBase_.translate.y <= targetY) {
            transformBase_.translate.y = targetY; // 接地点でぴったり止める

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
        // 地面に停止中
        velocity_ = { 0.0f, 0.0f, 0.0f };

        // ★ 「プレイヤーが投げた球」のみ一定時間経過で消滅処理へ
        if (thrownByPlayer_) {
            thrownGroundTimer_ += 1.0f / 60.0f;
            if (thrownGroundTimer_ >= kMaxThrownGroundTime_) {
                state_ = State::Disappearing;
                disappearTimer_ = 0.0f;
            }
        }
    }
    else if (state_ == State::Disappearing) {
        // 縮小しながら消滅
        disappearTimer_ += 1.0f / 60.0f;
        float t = std::min(disappearTimer_ / kDisappearTime_, 1.0f);
        float scale = (1.0f - t) * (1.0f - t);

        if (modelBody_) {
            modelBody_->transform.scale = { scale, scale, scale };
        }

        if (t >= 1.0f) {
            isDead_ = true; // GameSceneのクリーンアップで自動削除
        }
    }

    BaseCharacter::Update(activeCamera);
}