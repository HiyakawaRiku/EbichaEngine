#pragma once
#include "BaseCharacter.h"
#include "Physics3D.h"

class Player;

class HatSphere : public BaseCharacter
{
public:
    enum class State {
        OnGround,     // 地面に置かれている状態
        Equipped,     // プレイヤーの頭に乗っている状態
        Thrown,       // 投げられて飛んでいる状態
        Disappearing  // イージングで消滅中の状態
    };

    void Initialize(const Vector3& initialPos);
    void Update(Camera* activeCamera) override;

    // ★ リンカーエラーの原因となっている関数を宣言
    void EquipToPlayer(Player* player);
    void Throw(const Vector3& velocity);

    State GetState() const { return state_; }
    bool IsDead() const { return isDead_; }

    // 当たり判定（BSphere）を取得
    BSphere GetBSphere() const {
        return BSphere{ transformBase_.translate, radius_ };
    }

    // 衝突時のバウンド・反発処理
    void OnHit();

private:
    State state_ = State::OnGround;
    Player* ownerPlayer_ = nullptr;
    float headOffset_ = 1.8f;

    Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
    float gravity_ = 0.015f;
    float groundY_ = 0.5f;
    float bounceFriction_ = 0.5f;

    float radius_ = 0.5f;
    bool isDead_ = false;

    // 消滅イージング用タイマー
    float disappearTimer_ = 0.0f;
    const float kDisappearTime_ = 0.3f;
};