#include "EMath.h"
#include <cmath>
#include <numbers>

float EMath::LerpShortAngle(float a, float b, float t) {
    // C++20 の std::numbers::pi_v<float> または定義した π の定数を使用
    constexpr float kPi = std::numbers::pi_v<float>;
    constexpr float kTwoPi = kPi * 2.0f;

    float diff = b - a;

    diff = std::fmod(diff, kTwoPi);
    if (diff > kPi) {
        diff -= kTwoPi;
    }
    else if (diff < -kPi) {
        diff += kTwoPi;
    }

    // 最短距離の差分を使って補間
    return a + diff * t;
}
