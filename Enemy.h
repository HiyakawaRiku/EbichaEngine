#pragma once
#include "BaseCharacter.h"

class Enemy : public BaseCharacter
{
public:
    void Initialize() override;
    void Update(Camera* activeCamera) override;
    void Draw() override;
};