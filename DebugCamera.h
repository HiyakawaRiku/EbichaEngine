#pragma once
#include "Matrix.h"
#include "Camera.h"

class DebugCamera :public Camera{
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
    void Initialize()override;
    void Update()override;

    TransformationMatrix CalculateWVP(const Transform& objectTransform) override;

    Matrix4x4 GetViewMatrix() override { return viewMatrix_; }
    Matrix4x4 GetProjectionMatrix() override { return projectionMatrix_; }
};