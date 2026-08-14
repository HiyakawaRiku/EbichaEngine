#pragma once
#include "BaseCharacter.h"

class EnemyBullet : public BaseCharacter {
public:
    void Initialize(const Vector3& position, const Vector3& velocity);
    void Update(Camera* activeCamera) override;

    bool IsDead() const { return isDead_; }

private:
    Vector3 velocity_{};
    float deathTimer_ = 0.0f;
    static inline const float kLifeTime = 180.0f; // 3秒で消滅
    bool isDead_ = false;
};