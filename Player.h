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

    // =========================================================
    // HP & ダメージ関連機能
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

    AABB GetColliderAABB() const;

    // ImGui調整用アクセッサ
    Vector3& GetBoxExtents() { return boxExtents_; }
    void SetBoxExtents(const Vector3& extents) { boxExtents_ = extents; }

    // ハンマーの当たり判定（OBB）を取得
    OBB GetHammerColliderOBB() const;

    // ハンマーのOBBサイズ・オフセット調整用アクセッサ
    Vector3& GetHammerBoxExtents() { return hammerBoxExtents_; }
    Vector3& GetHammerColliderOffset() { return hammerColliderOffset_; }

    // 突進攻撃判定（OBB）を取得
    OBB GetDashAttackOBB() const;

    // 攻撃中判定（最高速度時の突進攻撃中か）
    bool IsAttacking() const { return isDashAttacking_; }
    bool IsDashAttacking() const { return isDashAttacking_; }

    bool IsJumping() const { return isJumping_; }
    float GetJumpVelocityY() const { return jumpVelocityY_; }

private:
    float floatingParameter_ = 0.0f;
    float frame_ = 60.0f;
    float floatingAmplitude = 0.1f;

    // --- HP・無敵時間用パラメータ ---
    static inline const int kMaxHp_ = 10;               // 最大HP
    int hp_ = kMaxHp_;                                 // 現在HP

    bool isInvincible_ = false;                        // 被弾後の無敵フラグ
    float invincibleTimer_ = 0.0f;                     // 無敵タイマー
    static inline const float kInvincibleTime = 60.0f; // 無敵時間（1秒 = 60フレーム）

    int hammerPartIndex_ = -1;

    // ImGuiでリアルタイム調整するためのオフセット用変数
    Vector3 hammerOffsetPos_ = { 0.0f, -0.2f, 0.4f };  // 手からの位置ズレ (X, Y, Z)
    Vector3 hammerOffsetRot_ = { 0.0f,  3.14f, 0.0f };  // 向きの角度 (X, Y, Z)

    Vector3 boxExtents_{ 0.5f, 1.3f, 0.5f };

    // ハンマー判定用のAABBサイズ（中心からのハーフサイズ）
    Vector3 hammerBoxExtents_{ 0.8f, 0.8f, 0.8f };

    // ハンマーモデル中心からの判定位置オフセット（X, Y, Z）
    Vector3 hammerColliderOffset_{ 0.0f, 0.0f, 0.0f };

    // --- ジャンプ調整用パラメータ ---
    float jumpInitialVelocity_ = 0.45f;   // 初速（少し高めに設定）
    float gravity_ = 0.015f;              // 上昇中の基本重力
    float fallGravityMultiplier_ = 2.0f;  // 下降時の重力倍率（キビキビ着地させる）
    float jumpCutMultiplier_ = 0.5f;      // ボタンを離したときの上昇減速倍率

    bool isJumping_ = false;
    float jumpVelocityY_ = 0.0f;
    float jumpGroundY_ = 1.0f;

    // --- ジャンプ変形・演出用パラメータ ---
    float landingBounceTimer_ = 0.0f;     // 着地時の潰れ・復元用タイマー
    bool isLandingBounce_ = false;        // 着地バウンド中フラグ
    float jumpStartRotX_ = 0.0f;          // ジャンプ開始時のX軸角度
    float jumpTargetRotX_ = 0.0f;         // 目標のX軸角度

    // --- 球体プレイヤー移動・慣性・ダッシュ突進攻撃用 ---
    Vector3 moveVelocity_ = { 0.0f, 0.0f, 0.0f }; // 現在の速度ベクトル
    static inline const float kTurnInertiaRate = 0.12f; // 振り向き時の追従補正率（値が小さいほど滑らかに振り向く）
    static inline const float kStopFriction = 0.85f;    // 立ち止まり時の減衰率（値が大きいほど遠くまで滑る）

    float sphereRollAngleX_ = 0.0f;       // X軸周りの転がり回転累積値
    float sphereRadius_ = 1.0f;           // 球体の半径

    float moveDashTimer_ = 0.0f;          // 移動長押し時間タイマー
    const float kBaseMoveSpeed = 0.15f;   // 初速（通常移動速度）
    const float kMaxMoveSpeed = 0.35f;    // 長押し時の最高速度
    const float kDashAccelTime = 600.0f;  // 最高速度に達するまでのフレーム数（約2秒）

    bool isDashAttacking_ = false;        // 最高速度時の突進攻撃状態フラグ
    Vector3 dashAttackExtents_{ 0.8f, 0.8f, 0.8f }; // 突進攻撃の判定サイズ

    // --- 待機モーション用パラメータ ---
    float idleMotionTimer_ = 0.0f;
    const float kIdleMotionSpeed = 0.05f;
};