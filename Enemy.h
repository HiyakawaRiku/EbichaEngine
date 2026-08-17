#pragma once
#include "BaseCharacter.h"
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>

class Enemy : public BaseCharacter
{
public:
    // 攻撃の種類（弾攻撃を除外）
    enum class AttackType {
        Charge,         // 1. 通常突進
        BouncingCharge, // 2. バウンド突進
        GigaSlam        // 3. ★ 大技：跳躍落下叩きつけ（後にひるむ）
    };

    // 敵の状態定義
    enum class State {
        Normal,     // 浮遊・移動
        AttackPrep, // 攻撃予備動作（溜め）
        Attacking,  // 攻撃実行中
        AttackCool  // 攻撃後の隙（クールダウン／ひるみ）
    };

    void Initialize();
    void Update(Camera* activeCamera) override;
    void Draw() override;

    bool IsAttacking() const { return state_ == State::Attacking; }

    // ★ 敵が大きくひるんでいるか（ダウン中か）の判定。プレイヤーのジャンプ攻撃チャンス！
    bool IsGroggy() const { return state_ == State::AttackCool && currentAttackType_ == AttackType::GigaSlam; }

    // プレイヤーの参照をセット（追尾・方向計算用）
    void SetTargetPlayer(const BaseCharacter* player) { targetPlayer_ = player; }

    // =========================================================
    // HP & ダメージ関連機能
    // =========================================================
    void TakeDamage(int damage);
    int GetHp() const { return hp_; }
    int GetMaxHp() const { return kMaxHp_; }
    bool IsDead() const { return hp_ <= 0; }

private:
    void UpdateNormal();
    void UpdateAttack();
    void UpdateFloating();

private:
    State state_ = State::Normal;
    AttackType currentAttackType_ = AttackType::Charge;

    const BaseCharacter* targetPlayer_ = nullptr;

    // HP・被弾用パラメータ
    static inline const int kMaxHp_ = 25;
    int hp_ = kMaxHp_;
    bool isInvincible_ = false;
    float invincibleTimer_ = 0.0f;
    static inline const float kInvincibleTime = 10.0f;

    // 浮遊用パラメータ
    float floatTimer_ = 0.0f;
    static inline const float kBaseHeight = 2.5f;
    static inline const float kFloatSpeed = 0.05f;
    static inline const float kFloatAmplitude = 0.3f;

    // ターゲット位置・方向保持用
    Vector3 chargeDirection_{ 0.0f, 0.0f, 1.0f };
    Vector3 slamTargetPos_{ 0.0f, 0.0f, 0.0f }; // 大技の落地点

    // タイマー関連
    float attackIntervalTimer_ = 0.0f;
    float attackStateTimer_ = 0.0f;

    // 定数パラメータ
    static inline const float kAttackInterval = 140.0f; // 攻撃間隔
    static inline const float kPrepDuration = 40.0f;    // 通常溜め時間
    static inline const float kAttackDuration = 20.0f;  // 通常攻撃時間
    static inline const float kCoolDuration = 45.0f;    // 通常硬直時間

    static inline const float kAttackDashSpeed = 0.5f;
};