#include "Enemy.h"
#include <cmath>
#include <cstdlib>

#ifdef _DEBUG
#include "imgui.h"
#endif

void Enemy::Initialize()
{
    std::vector<BaseCharacter::PartConfig> partConfigs = {};
    BaseCharacter::Initialize("cube", partConfigs, "resources/white1x1.png");

    transformBase_.translate = { 3.0f, kBaseHeight, 0.0f };
    SetColliderRadius(1.2f);

    state_ = State::Normal;
    attackIntervalTimer_ = 0.0f;
    attackStateTimer_ = 0.0f;
    floatTimer_ = 0.0f;

    hp_ = kMaxHp_;
    isInvincible_ = false;
    invincibleTimer_ = 0.0f;
    spawnRequests_.clear();
}

void Enemy::Update(Camera* activeCamera)
{
    if (IsDead()) {
        return;
    }

    if (isInvincible_) {
        invincibleTimer_ -= 1.0f;
        if (invincibleTimer_ <= 0.0f) {
            isInvincible_ = false;
            invincibleTimer_ = 0.0f;
        }
    }

    UpdateFloating();

    switch (state_) {
    case State::Normal:
        UpdateNormal();
        break;
    case State::AttackPrep:
    case State::AttackLock:
    case State::Attacking:
    case State::AttackCool:
        UpdateAttack();
        break;
    }

    BaseCharacter::Update(activeCamera);
}

void Enemy::UpdateFloating()
{
    if (state_ == State::Normal || state_ == State::AttackCool) {
        floatTimer_ += kFloatSpeed;
        float offset = std::sin(floatTimer_) * kFloatAmplitude;
        transformBase_.translate.y = kBaseHeight + offset;
    }
}

void Enemy::UpdateNormal()
{
    if (targetPlayer_) {
        Vector3 playerPos = targetPlayer_->GetTransform().translate;
        Vector3 diff = playerPos - transformBase_.translate;
        float targetAngle = std::atan2(diff.x, diff.z);

        modelBody_->transform.rotate.y = EMath::LerpShortAngle(
            modelBody_->transform.rotate.y, targetAngle, 0.1f
        );
    }

    attackIntervalTimer_ += 1.0f;
    if (attackIntervalTimer_ >= kAttackInterval) {
        state_ = State::AttackPrep;
        attackIntervalTimer_ = 0.0f;
        attackStateTimer_ = 0.0f;

        // ランダムに攻撃パターンを選択 (4種類)
        int type = rand() % 4;
        currentAttackType_ = static_cast<AttackType>(type);
    }
}

void Enemy::UpdateAttack()
{
    attackStateTimer_ += 1.0f;

    switch (state_) {
    case State::AttackPrep:
    {
        float prepDuration = (currentAttackType_ == AttackType::GigaSlam) ? 50.0f : kPrepDuration;

        if (currentAttackType_ == AttackType::GigaSlam) {
            float progress = attackStateTimer_ / prepDuration;
            transformBase_.translate.y = kBaseHeight + progress * 6.0f;
            modelBody_->transform.scale = { 1.0f + progress * 1.0f, 1.0f + progress * 1.0f, 1.0f + progress * 1.0f };
            modelBody_->transform.rotate.y += 0.2f;

            if (targetPlayer_) {
                slamTargetPos_ = targetPlayer_->GetTransform().translate;
                slamTargetPos_.y = 0.05f;
            }
        }
        else if (currentAttackType_ == AttackType::SpawnHatSphere) {
            // Cube吐き出し前兆（上下にぐにゅっと伸縮）
            float prepProgress = attackStateTimer_ / prepDuration;
            float pulse = std::sin(prepProgress * 3.14159f * 4.0f) * 0.2f;
            modelBody_->transform.scale = { 1.0f - pulse, 1.0f + pulse, 1.0f - pulse };
        }
        else if (currentAttackType_ == AttackType::Charge || currentAttackType_ == AttackType::BouncingCharge) {
            modelBody_->transform.rotate.x = -0.4f;
            modelBody_->transform.scale = { 0.9f, 0.8f, 1.1f };
        }
        else {
            modelBody_->transform.scale = { 1.4f, 1.4f, 1.4f };
        }

        if (attackStateTimer_ >= prepDuration) {
            if (currentAttackType_ == AttackType::GigaSlam) {
                state_ = State::AttackLock;
            }
            else {
                state_ = State::Attacking;
            }
            attackStateTimer_ = 0.0f;

            if (targetPlayer_) {
                Vector3 diff = targetPlayer_->GetTransform().translate - transformBase_.translate;
                diff.y = 0.0f;
                float len = std::sqrt(diff.x * diff.x + diff.z * diff.z);
                chargeDirection_ = (len > 0.0f) ? Vector3{ diff.x / len, 0.0f, diff.z / len } : Vector3{ 0.0f, 0.0f, 1.0f };
            }
        }
    }
    break;

    case State::AttackLock:
    {
        float lockDuration = kLockDuration;

        if (currentAttackType_ == AttackType::GigaSlam) {
            float shake = std::sin(attackStateTimer_ * 1.5f) * 0.1f;
            transformBase_.translate.x += shake;
            modelBody_->transform.rotate.y += 0.4f;
        }

        if (attackStateTimer_ >= lockDuration) {
            state_ = State::Attacking;
            attackStateTimer_ = 0.0f;
        }
    }
    break;

    case State::Attacking:
    {
        float currentDuration = kAttackDuration;

        if (currentAttackType_ == AttackType::GigaSlam) {
            currentDuration = 15.0f;
            float progress = attackStateTimer_ / currentDuration;

            transformBase_.translate.x += (slamTargetPos_.x - transformBase_.translate.x) * 0.3f;
            transformBase_.translate.z += (slamTargetPos_.z - transformBase_.translate.z) * 0.3f;
            transformBase_.translate.y = (kBaseHeight + 6.0f) * (1.0f - progress) + 0.5f * progress;

            if (attackStateTimer_ >= currentDuration - 1.0f) {
                transformBase_.translate.y = 0.5f;
            }
        }
        else if (currentAttackType_ == AttackType::SpawnHatSphere) {
            currentDuration = 32.0f;

            // 8フレームごとに計3個射出
            int frame = static_cast<int>(attackStateTimer_);
            if (frame % 8 == 1 && frame <= 25) {
                Vector3 spawnPos = transformBase_.translate;
                spawnPos.y += 1.2f;

                float randSpread = ((rand() % 100) / 100.0f - 0.5f) * 0.6f;
                float baseAngle = std::atan2(chargeDirection_.x, chargeDirection_.z) + randSpread;

                float forwardSpeed = 0.35f + ((rand() % 100) / 100.0f) * 0.15f;
                float upPower = 0.25f;

                Vector3 vel = {
                    std::sin(baseAngle) * forwardSpeed,
                    upPower,
                    std::cos(baseAngle) * forwardSpeed
                };

                spawnRequests_.push_back({ spawnPos, vel });
            }

            modelBody_->transform.scale = { 1.2f, 0.8f, 1.2f };
        }
        else if (currentAttackType_ == AttackType::Charge) {
            transformBase_.translate += chargeDirection_ * kAttackDashSpeed;
        }
        else if (currentAttackType_ == AttackType::BouncingCharge) {
            currentDuration = kAttackDuration * 2.0f;
            transformBase_.translate += chargeDirection_ * (kAttackDashSpeed * 0.8f);

            float bounceHeight = std::abs(std::sin(attackStateTimer_ * 0.2f)) * 3.0f;
            transformBase_.translate.y = kBaseHeight + bounceHeight;
            modelBody_->transform.rotate.x += 0.3f;
        }

        if (attackStateTimer_ >= currentDuration) {
            state_ = State::AttackCool;
            attackStateTimer_ = 0.0f;
        }
    }
    break;

    case State::AttackCool:
    {
        isShowWarning_ = false;
        float coolDuration = kCoolDuration;

        if (currentAttackType_ == AttackType::GigaSlam) {
            coolDuration = 180.0f;
            modelBody_->transform.scale = { 1.6f, 0.3f, 1.6f };
            modelBody_->transform.rotate.x = 0.5f;
            transformBase_.translate.y = 0.3f;
        }
        else {
            modelBody_->transform.rotate.x = 0.0f;
            modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };
        }

        if (attackStateTimer_ >= coolDuration) {
            state_ = State::Normal;
            attackStateTimer_ = 0.0f;
            modelBody_->transform.rotate.x = 0.0f;
            modelBody_->transform.scale = { 1.0f, 1.0f, 1.0f };
        }
    }
    break;
    }
}

void Enemy::TakeDamage(int damage)
{
    if (isInvincible_) return;

    hp_ -= damage;
    if (hp_ < 0) hp_ = 0;

    isInvincible_ = true;
    invincibleTimer_ = kInvincibleTime;
}

void Enemy::Draw()
{
    if (IsDead()) return;

    BaseCharacter::Draw();
}

void Enemy::SpawnHatSpheresRandomly(int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_real_distribution<float> distAngle(0.0f, 6.28318f);
    std::uniform_real_distribution<float> distSpeedXZ(0.2f, 0.4f); // 速度を少し速めに
    std::uniform_real_distribution<float> distSpeedY(0.2f, 0.35f);

    Vector3 enemyPos = transformBase_.translate;

    for (int i = 0; i < count; ++i) {
        float angle = distAngle(gen);
        float speedXZ = distSpeedXZ(gen);
        float speedY = distSpeedY(gen);

        Vector3 velocity = {
            std::cos(angle) * speedXZ,
            speedY,
            std::sin(angle) * speedXZ
        };

        // ★ 敵の半径(1.2f)より外側から生成して当たり判定のめり込みを防ぐ
        Vector3 spawnPos = {
            enemyPos.x + std::cos(angle) * 2.0f,
            enemyPos.y + 0.5f,
            enemyPos.z + std::sin(angle) * 2.0f
        };

        spawnRequests_.push_back({ spawnPos, velocity });
    }
}