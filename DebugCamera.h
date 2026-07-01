#pragma once
#include "Matrix.h"

class DebugCamera {
private:
    // ローカル座標
    Vector3 translation_ = { 0.0f, 0.0f, -50.0f };
    // 累積回転行列
    Matrix4x4 matRot_ = MakeIdentity4x4();

    // ビュー行列
    Matrix4x4 viewMatrix_;
    // 射影行列
    Matrix4x4 projectionMatrix_;

public:
    void Initialize();
    void Update();

    const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
    const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
};