#include "Enemy.h"
#include <cmath>

void Enemy::Initialize()
{
    // 敵用にパーツ無し（または手足を配置）で初期化[cite: 5]
    // ※ Player のパーツ（Body.obj 等）を流用する場合は partConfigs にパーツを追加してください[cite: 5]
    std::vector<BaseCharacter::PartConfig> partConfigs = {};

    // BaseCharacter の初期化を呼び出す[cite: 5]
    BaseCharacter::Initialize("enemy.obj", partConfigs, "resources/player.png"); //[cite: 5]

    transformBase_.translate = { 5.0f, 0.0f, 0.0f }; //[cite: 7]
}

void Enemy::Update(Camera* activeCamera)
{
    // 親クラスの共通更新処理を実行（カメラ更新、モデル更新、行列更新）[cite: 5]
    BaseCharacter::Update(activeCamera);

    // ----------------------------------------------------
    // Enemy固有の移動・アニメーション処理[cite: 7]
    // ----------------------------------------------------
    Vector3 moveVelocity = { 0.0f, 0.0f, 0.0f }; //[cite: 7]
    // ※AI追尾などのロジックを入れる場合はここで moveVelocity を計算

    bool isMoving = (moveVelocity.x != 0.0f || moveVelocity.y != 0.0f || moveVelocity.z != 0.0f); //[cite: 7]

    if (isMoving) { //[cite: 7]
        walkTimer_ += kWalkSpeed; //[cite: 7]
        float sinVal = std::sin(walkTimer_); //[cite: 7]

        modelBody_->transform.translate.y = std::abs(sinVal) * kWalkHopHeight; //[cite: 7]
        modelBody_->transform.rotate.x = kWalkTilt; //[cite: 7]

        float targetAngle = std::atan2(moveVelocity.x, moveVelocity.z); //[cite: 7]
        modelBody_->transform.rotate.y = targetAngle; //[cite: 7]

        transformBase_.translate += moveVelocity; //[cite: 7]
    }
    else { //[cite: 7]
        walkTimer_ = 0.0f; //[cite: 7]
        idleTimer_ += kIdleSpeed; //[cite: 7]
        float sinVal = std::sin(idleTimer_); //[cite: 7]

        modelBody_->transform.translate.y = sinVal * kIdleBreathing; //[cite: 7]

        modelBody_->transform.scale.x = 1.0f - (sinVal * kIdleSquash); //[cite: 7]
        modelBody_->transform.scale.y = 1.0f + (sinVal * kIdleSquash); //[cite: 7]
        modelBody_->transform.scale.z = 1.0f - (sinVal * kIdleSquash); //[cite: 7]

        modelBody_->transform.rotate.x = 0.0f; //[cite: 7]
    }
}