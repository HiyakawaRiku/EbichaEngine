#pragma once
#include "EMath.h"

struct Transform
{
    // SRTデータ
    Vector3 scale = { 1.0f, 1.0f, 1.0f };
    Vector3 rotate = { 0.0f, 0.0f, 0.0f };
    Vector3 translate = { 0.0f, 0.0f, 0.0f };

    // 親Transformへのポインタ（NULLなら親なし）
    const Transform* parent = nullptr;

    // 計算後のワールド行列
    Matrix4x4 matWorld;

    // 初期化関数
    void Initialize() {
        scale = { 1.0f, 1.0f, 1.0f };
        rotate = { 0.0f, 0.0f, 0.0f };
        translate = { 0.0f, 0.0f, 0.0f };
        parent = nullptr;
        matWorld = MakeIdentity4x4();
    }

    // 行列更新処理
    void UpdateMatrix() {
        // 1. 自身のSRTからローカル行列（MakeAffineMatrix）を合成
        Matrix4x4 matLocal = MakeAffineMatrix(scale, rotate, translate);

        // 2. 親が存在する場合は、親のワールド行列を乗算する
        if (parent) {
            matWorld = Multiply(matLocal, parent->matWorld);
        }
        else {
            matWorld = matLocal;
        }
    }
};