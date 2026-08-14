#pragma once
#include "BaseCharacter.h"
#include "EnemyBullet.h"
#include <random>

class Enemy : public BaseCharacter
{
public:
    // 攻撃の種類
    enum class AttackType {
        Charge, // 突進攻撃
        Shoot   // 弾発射攻撃
    };

    // 敵の状態定義
    enum class State {
        Normal,     // 浮遊・移動
        AttackPrep, // 攻撃予備動作（溜め）
        Attacking,  // 攻撃実行中
        AttackCool  // 攻撃後の隙（クールダウン）
    };

    void Initialize();
    void Update(Camera* activeCamera) override;
    void Draw() override; // 弾描画のためにOverride

    bool IsAttacking() const { return state_ == State::Attacking; }

    // プレイヤーの参照をセット（追尾・射撃方向計算用）
    void SetTargetPlayer(const BaseCharacter* player) { targetPlayer_ = player; }

    // 敵が発射した弾リストの参照取得
    const std::vector<std::unique_ptr<EnemyBullet>>& GetBullets() const { return bullets_; }

private:
    void UpdateNormal();
    void UpdateAttack();
    void UpdateFloating(); // 常に浮遊させる関数
    void FireBullet();     // 弾発射処理

private:
    State state_ = State::Normal;
    AttackType currentAttackType_ = AttackType::Charge;

    const BaseCharacter* targetPlayer_ = nullptr; // プレイヤーの参照

    // 弾の一括管理
    std::vector<std::unique_ptr<EnemyBullet>> bullets_;

    // 浮遊用パラメータ
    float floatTimer_ = 0.0f;
    static inline const float kBaseHeight = 2.5f;     // 基本の浮遊高度 (Y座標)
    static inline const float kFloatSpeed = 0.05f;    // 浮遊の上下揺れスピード
    static inline const float kFloatAmplitude = 0.3f; // 浮遊の揺れ幅

    // 突進時のターゲット方向保持用
    Vector3 chargeDirection_{ 0.0f, 0.0f, 1.0f };

    // タイマー関連
    float attackIntervalTimer_ = 0.0f;
    float attackStateTimer_ = 0.0f;

    // 定数パラメータ
    static inline const float kAttackInterval = 150.0f; // 攻撃間隔
    static inline const float kPrepDuration = 40.0f;    // 溜め時間
    static inline const float kAttackDuration = 20.0f;  // 攻撃時間
    static inline const float kCoolDuration = 45.0f;    // 硬直時間

    static inline const float kAttackDashSpeed = 0.5f;  // 突進速度
    static inline const float kBulletSpeed = 0.4f;      // 弾速
};