#include "ParticleEmitter.h"
#include "ParticleManager.h"

ParticleEmitter::ParticleEmitter(const std::string& name, const Vector3& position, uint32_t count, float frequency)
    : name_(name), emitCount(count), emitFrequency(frequency)
{
    transform.translate = position;
    transform.scale = { 1.0f, 1.0f, 1.0f };
    transform.rotate = { 0.0f, 0.0f, 0.0f };
}

void ParticleEmitter::Update()
{
    // デルタタイム想定 (60FPS基準 1/60秒)
    const float kDeltaTime = 1.0f / 60.0f;
    deltaTimer += kDeltaTime;

    if (deltaTimer >= emitFrequency) {
        Emit();
        deltaTimer -= emitFrequency;
    }
}

void ParticleEmitter::Emit()
{
    ParticleManager::GetInstance()->Emit(name_, transform.translate, emitCount);
}