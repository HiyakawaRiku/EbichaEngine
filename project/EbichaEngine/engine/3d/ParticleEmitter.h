#pragma once
#include "EMath.h"
#include <string>

class ParticleEmitter
{
public:
    ParticleEmitter(const std::string& name, const Vector3& position, uint32_t count = 10, float frequency = 0.5f);

    void Update();
    void Emit();

    void SetPosition(const Vector3& position) { transform.translate = position; }
    void SetEmitCount(uint32_t count) { emitCount = count; }
    void SetFrequency(float frequency) { emitFrequency = frequency; }

private:
    std::string name_;
    Transform transform{};
    uint32_t emitCount = 10;

    float emitFrequency = 0.5f; // 発生間隔(秒)
    float deltaTimer = 0.0f;
};