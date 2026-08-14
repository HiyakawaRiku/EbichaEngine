#pragma once
#include "BaseCharacter.h"

class Enemy : public BaseCharacter
{
public:
    void Initialize();
    void Update(Camera* activeCamera) override;
    // ※ Draw() は BaseCharacter::Draw() を使用するため宣言不要[cite: 5, 6]

private:
    // 単一モデル用の伸縮パラメータ（単一モデル enemy.obj を使用する場合）
    static inline const float kWalkHopHeight = 0.3f;
    static inline const float kWalkTilt = 0.1f;
    static inline const float kIdleSquash = 0.05f;
};