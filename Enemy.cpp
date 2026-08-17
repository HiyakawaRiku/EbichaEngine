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
    bullets_.clear();

    hp_ = kMaxHp_;
    isInvincible_ = false;
    invincibleTimer_ = 0.0f;
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

#ifdef _DEBUG
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 150.0f, 20.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 85.0f));
    ImGui::Begin("Enemy Status", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("ENEMY HP: %d / %d", hp_, kMaxHp_);

    if (IsGroggy()) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "!! CHANCE !! GROGGY (JUMP ATTACK!)");
    }
    else {
        ImGui::Text("State: Active");
    }

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IsGroggy() ? ImVec4(0.9f, 0.7f, 0.1f, 1.0f) : ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
    ImGui::ProgressBar((float)hp_ / (float)kMaxHp_, ImVec2(-1.0f, 20.0f), "");
    ImGui::PopStyleColor();

    ImGui::End();
#endif
}

void Enemy::Draw()
{
    if (!IsDead()) {
        BaseCharacter::Draw();
    }

    for (const auto& bullet : bullets_) {
        bullet->Draw();
    }
}

void Enemy::TakeDamage(int damage)
{
    if (isInvincible_ || IsDead()) return;

    // ★ ひるみ（ダウン）中にジャンプ攻撃を受けた場合は特大ダメージ（2.5倍）
    if (IsGroggy()) {
        damage = static_cast<int>(damage * 2.5f);
    }

    hp_ -= damage;
    if (hp_ < 0) hp_ = 0;

    if (targetPlayer_) {
        Vector3 playerPos = targetPlayer_->GetTransform().translate;
        Vector3 pushDir = transformBase_.translate - playerPos;
        pushDir.y = 0.0f;

        float len = std::sqrt(pushDir.x * pushDir.x + pushDir.z * pushDir.z);
        if (len > 0.0f) {
            transformBase_.translate += (pushDir / len) * 0.8f;
        }
    }

    isInvincible_ = true;
    invincibleTimer_ = kInvincibleTime;
}

void Enemy::UpdateFloating()
{
    floatTimer_ += kFloatSpeed;
    float sinVal = std::sin(floatTimer_);

    // Normal時または大技以外の時に基準の高さで浮遊
    if (state_ == State::Normal) {
        transformBase_.translate.y = kBaseHeight + sinVal * kFloatAmplitude;
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

        // 6種類の攻撃からランダム選択（大技の割合を高めに）
        int type = rand() % 6;
        currentAttackType_ = static_cast<AttackType>(type);
    }
}

void Enemy::UpdateAttack()
{
    attackStateTimer_ += 1.0f;

    switch (state_) {
    case State::AttackPrep:
    {
        float prepDuration = kPrepDuration;

        if (currentAttackType_ == AttackType::GigaSlam) {
            // 【大技溜め】上空へ上昇しつつ体を大増殖・巨大化
            prepDuration = 60.0f; // 溜め時間（長め）
            float progress = attackStateTimer_ / prepDuration;

            transformBase_.translate.y = kBaseHeight + progress * 6.0f; // 上空へ舞い上がる
            modelBody_->transform.scale = { 1.0f + progress * 1.0f, 1.0f + progress * 1.0f, 1.0f + progress * 1.0f }; // 2倍の大きさに
            modelBody_->transform.rotate.y += 0.2f;

            // ターゲット（プレイヤー位置）の保持
            if (targetPlayer_) {
                slamTargetPos_ = targetPlayer_->GetTransform().translate;
                slamTargetPos_.y = 0.5f; // 地面着地点
            }
        }
        else if (currentAttackType_ == AttackType::Charge || currentAttackType_ == AttackType::BouncingCharge) {
            modelBody_->transform.rotate.x = -0.4f;
            modelBody_->transform.scale = { 0.9f, 0.8f, 1.1f };
        }
        else if (currentAttackType_ == AttackType::SpinShoot) {
            modelBody_->transform.rotate.y += 0.3f;
            modelBody_->transform.scale = { 1.2f, 0.8f, 1.2f };
        }
        else {
            modelBody_->transform.scale = { 1.4f, 1.4f, 1.4f };
        }

        if (attackStateTimer_ >= prepDuration) {
            state_ = State::Attacking;
            attackStateTimer_ = 0.0f;

            if (targetPlayer_) {
                Vector3 diff = targetPlayer_->GetTransform().translate - transformBase_.translate;
                diff.y = 0.0f;
                float len = std::sqrt(diff.x * diff.x + diff.z * diff.z);
                if (len > 0.0f) {
                    chargeDirection_ = { diff.x / len, 0.0f, diff.z / len };
                }
                else {
                    chargeDirection_ = { 0.0f, 0.0f, 1.0f };
                }
            }

            if (currentAttackType_ == AttackType::Shoot) {
                FireBullet();
            }
            else if (currentAttackType_ == AttackType::RingShoot) {
                FireRingBullet(16);
            }
        }
    }
    break;

    case State::Attacking:
    {
        float currentDuration = kAttackDuration;

        if (currentAttackType_ == AttackType::GigaSlam) {
            // 【大技実行】プレイヤー位置へ向かって超高速急降下叩きつけ
            currentDuration = 15.0f;
            float progress = attackStateTimer_ / currentDuration;

            // 補間移動で着地させる
            transformBase_.translate.x += (slamTargetPos_.x - transformBase_.translate.x) * 0.3f;
            transformBase_.translate.z += (slamTargetPos_.z - transformBase_.translate.z) * 0.3f;
            transformBase_.translate.y = (kBaseHeight + 6.0f) * (1.0f - progress) + 0.5f * progress;

            // 着地した瞬間（攻撃の終わり）に全方向に超強力な拡散衝撃波（18方向弾幕）を発射
            if (attackStateTimer_ >= currentDuration - 1.0f) {
                transformBase_.translate.y = 0.5f; // 地面叩きつけ
                FireRingBullet(18); // 着地衝撃波
            }
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
        else if (currentAttackType_ == AttackType::SpinShoot) {
            currentDuration = kAttackDuration * 2.5f;
            modelBody_->transform.rotate.y += 0.4f;

            if (static_cast<int>(attackStateTimer_) % 3 == 0) {
                float angle = modelBody_->transform.rotate.y;
                Vector3 dir = { std::sin(angle), 0.0f, std::cos(angle) };
                FireSingleBullet(dir, kBulletSpeed * 0.8f);
            }
        }

        if (attackStateTimer_ >= currentDuration) {
            state_ = State::AttackCool;
            attackStateTimer_ = 0.0f;
        }
    }
    break;

    case State::AttackCool:
    {
        float coolDuration = kCoolDuration;

        // 【ひるみ状態処理】大技「GigaSlam」の後は長いスキ（ひるみ／ダウン）を作る
        if (currentAttackType_ == AttackType::GigaSlam) {
            coolDuration = 180.0f; // 3秒間無防備なひるみ状態になる（攻撃の大チャンス）

            // 体をぺしゃんこに潰して地面に伏せる（ひるみ演出）
            modelBody_->transform.scale = { 1.6f, 0.3f, 1.6f };
            modelBody_->transform.rotate.x = 0.5f; // 傾いてぐったりする
            transformBase_.translate.y = 0.3f;     // 低い姿勢
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

void Enemy::FireBullet()
{
    Vector3 bulletVel = chargeDirection_ * kBulletSpeed;
    auto bullet = std::make_unique<EnemyBullet>();
    bullet->Initialize(transformBase_.translate, bulletVel);
    bullets_.push_back(std::move(bullet));
}

void Enemy::FireSingleBullet(const Vector3& dir, float speed)
{
    Vector3 bulletVel = dir * speed;
    auto bullet = std::make_unique<EnemyBullet>();
    bullet->Initialize(transformBase_.translate, bulletVel);
    bullets_.push_back(std::move(bullet));
}

void Enemy::FireRingBullet(int count)
{
    const float kTwoPi = 6.28318530718f;
    for (int i = 0; i < count; ++i) {
        float angle = (kTwoPi / count) * static_cast<float>(i);

        Vector3 dir = {
            std::sin(angle),
            0.0f,
            std::cos(angle)
        };

        FireSingleBullet(dir, kBulletSpeed * 0.7f);
    }
}