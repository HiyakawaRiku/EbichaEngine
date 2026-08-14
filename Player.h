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

    // ★ 攻撃処理用の関数を追加
    void BehaviorAttackInitialize();
    void BehaviorAttackUpdate();

    // ★ 他のオブジェクト（敵など）から攻撃中かどうか判定するためのゲッター
    bool IsAttacking() const { return isAttacking_; }

private:
    static inline const float kAcceleration = 0.2f;

    float floatingParameter_ = 0.0f;
    float frame_ = 60.0f;
    float floatingAmplitude = 0.1f;

    // --- 攻撃アニメーション用 ---
    bool isAttacking_ = false;
    float attackTimer_ = 0.0f;
    const float kAttackDuration = 40.0f; // 攻撃モーションの長さ（フレーム数）

    // --- ジャンプアニメーション用 ---
    float jumpTimer_ = 0.0f;
    float jumpVelocityY_ = 0.0f;
    bool isJumping_ = false;

    // ImGui調整可能パラメータ
    float jumpInitialVelocity_ = 0.4f;
    float gravity_ = 0.02f;
    float jumpSquashAmount_ = 0.3f;
    float jumpGroundY_ = 1.5f;
};