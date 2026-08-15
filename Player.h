#pragma once
#include "BaseCharacter.h"

class Player : public BaseCharacter
{
public:
    void Initialize();
    void Update(Camera* activeCamera) override;

    void InitializeFloatingGimmick();
    void UpdateFloatingGimmick();

    void BehaviorRootUpdate(Camera* activeCamera_);
    void BehaviorJumpInitialize();
    void BehaviorJumpUpdate();

    // 攻撃処理
    void BehaviorAttackInitialize();
    void BehaviorAttackUpdate();

    bool IsAttacking() const { return isAttacking_; }

    // =========================================================
    // ★ HP & ダメージ関連機能
    // =========================================================

    /// <summary>
    /// ダメージを受ける処理
    /// </summary>
    /// <param name="damage">受けるダメージ量</param>
    void TakeDamage(int damage);

    // HP 関連のゲッター
    int GetHp() const { return hp_; }
    int GetMaxHp() const { return kMaxHp_; }
    bool IsDead() const { return hp_ <= 0; }
    bool IsInvincible() const { return isInvincible_; }

private:
    static inline const float kAcceleration = 0.2f;

    float floatingParameter_ = 0.0f;
    float frame_ = 60.0f;
    float floatingAmplitude = 0.1f;

    // --- 攻撃アニメーション用 ---
    bool isAttacking_ = false;
    float attackTimer_ = 0.0f;
    const float kAttackDuration = 40.0f;

    // --- ジャンプアニメーション用 ---
    float jumpTimer_ = 0.0f;
    float jumpVelocityY_ = 0.0f;
    bool isJumping_ = false;

    // --- ★ HP・無敵時間用パラメータ ---
    static inline const int kMaxHp_ = 10;            // 最大HP
    int hp_ = kMaxHp_;                              // 現在HP

    bool isInvincible_ = false;                     // 被弾後の無敵フラグ
    float invincibleTimer_ = 0.0f;                  // 無敵タイマー
    static inline const float kInvincibleTime = 60.0f; // 無敵時間（1秒 = 60フレーム）

    // ImGui調整可能パラメータ
    float jumpInitialVelocity_ = 0.4f;
    float gravity_ = 0.02f;
    float jumpSquashAmount_ = 0.3f;
    float jumpGroundY_ = 1.5f;
};