#include "Enemy.h"
#include <cmath>

void Enemy::Initialize()
{
    std::vector<BaseCharacter::PartConfig> partConfigs = {};
    BaseCharacter::Initialize("enemy.obj", partConfigs, "resources/player.png");

    // 初期位置（浮かせた高さに設定）
    transformBase_.translate = { 3.0f, kBaseHeight, 0.0f };

    SetColliderRadius(1.2f);

    state_ = State::Normal;
    attackIntervalTimer_ = 0.0f;
    attackStateTimer_ = 0.0f;
    floatTimer_ = 0.0f;
    bullets_.clear();
}

void Enemy::Update(Camera* activeCamera)
{
    // 常に浮遊アニメーションを実行
    UpdateFloating();

    // 弾の更新＆死んだ弾の削除
    for (auto it = bullets_.begin(); it != bullets_.end(); ) {
        (*it)->Update(activeCamera);
        if ((*it)->IsDead()) {
            it = bullets_.erase(it);
        }
        else {
            ++it;
        }
    }

    switch (state_) {
    case State::Normal:
        UpdateNormal();
        break;
    case State::AttackPrep:
    case State::Attacking:
    case State::AttackCool:
        UpdateAttack();
        break;
    }

    BaseCharacter::Update(activeCamera);
}

void Enemy::Draw()
{
    BaseCharacter::Draw();

    // 発射した弾の描画
    for (const auto& bullet : bullets_) {
        bullet->Draw();
    }
}

void Enemy::UpdateFloating()
{
    // Y座標に正弦波（sin）を適用して常に空中を浮遊させる
    floatTimer_ += kFloatSpeed;
    float sinVal = std::sin(floatTimer_);

    // Normal時以外（攻撃中など）も基準の高さを浮遊
    if (state_ == State::Normal) {
        transformBase_.translate.y = kBaseHeight + sinVal * kFloatAmplitude;
    }
}

void Enemy::UpdateNormal()
{
    // プレイヤーの方向を向く
    if (targetPlayer_) {
        Vector3 playerPos = targetPlayer_->GetTransform().translate;
        Vector3 diff = playerPos - transformBase_.translate;
        float targetAngle = std::atan2(diff.x, diff.z);

        // 緩やかにプレイヤーの方へ回転
        modelBody_->transform.rotate.y = EMath::LerpShortAngle(
            modelBody_->transform.rotate.y, targetAngle, 0.1f
        );
    }

    // 攻撃までのカウントダウン
    attackIntervalTimer_ += 1.0f;
    if (attackIntervalTimer_ >= kAttackInterval) {
        state_ = State::AttackPrep;
        attackIntervalTimer_ = 0.0f;
        attackStateTimer_ = 0.0f;

        // 攻撃パターンをランダムで選択（突進 or 弾発射）
        currentAttackType_ = (rand() % 2 == 0) ? AttackType::Charge : AttackType::Shoot;
    }
}

void Enemy::UpdateAttack()
{
    attackStateTimer_ += 1.0f;

    switch (state_) {
    case State::AttackPrep:
        // --- 【予備動作】 ---
        if (currentAttackType_ == AttackType::Charge) {
            // 突進溜め: 後ろに引く
            modelBody_->transform.rotate.x = -0.4f;
            modelBody_->transform.scale = { 0.9f, 0.8f, 1.1f };
        }
        else {
            // 弾発射溜め: 体を膨らませて少し浮き上がる
            modelBody_->transform.scale = { 1.3f, 1.3f, 1.3f };
            transformBase_.translate.y += 0.02f;
        }

        // 溜め終了直前に突進方向（プレイヤーの位置）をロックオン
        if (attackStateTimer_ >= kPrepDuration) {
            state_ = State::Attacking;
            attackStateTimer_ = 0.0f;

            if (targetPlayer_) {
                Vector3 diff = targetPlayer_->GetTransform().translate - transformBase_.translate;
                float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
                if (len > 0.0f) {
                    chargeDirection_ = { diff.x / len, diff.y / len, diff.z / len };
                }
            }

            // 弾発射攻撃の場合は、攻撃開始瞬間に弾を生成
            if (currentAttackType_ == AttackType::Shoot) {
                FireBullet();
            }
        }
        break;

    case State::Attacking:
        // --- 【攻撃実行】 ---
        if (currentAttackType_ == AttackType::Charge) {
            // 突進: 計算した方向ベクトルに向かって高速移動
            modelBody_->transform.rotate.x = 0.5f;
            transformBase_.translate += chargeDirection_ * kAttackDashSpeed;
        }
        else {
            // 弾発射: 反動で少し後ろに下がる演出
            modelBody_->transform.rotate.x = -0.2f;
            modelBody_->transform.scale = { 0.9f, 0.9f, 0.9f };
        }

        if (attackStateTimer_ >= kAttackDuration) {
            state_ = State::AttackCool;
            attackStateTimer_ = 0.0f;
        }
        break;

    case State::AttackCool:
        // --- 【隙・クールダウン】 ---
        modelBody_->transform.rotate.x = 0.0f;
        modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };

        if (attackStateTimer_ >= kCoolDuration) {
            state_ = State::Normal;
            attackStateTimer_ = 0.0f;
        }
        break;
    }
}

void Enemy::FireBullet()
{
    // プレイヤーに向かう速度ベクトルを計算
    Vector3 bulletVel = chargeDirection_ * kBulletSpeed;

    auto bullet = std::make_unique<EnemyBullet>();
    bullet->Initialize(transformBase_.translate, bulletVel);
    bullets_.push_back(std::move(bullet));
}