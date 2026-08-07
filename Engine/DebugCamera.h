#pragma once
#include "Camera.h"

#include "Input.h"

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

    POINT prevMousePos_ = { 0, 0 }; // 前フレームのマウス位置
    bool isFirstFrame_ = true;      // 起動直後のズレ防止フラグ

    Vector3 targetPos_ = { 0.0f, 0.0f, 0.0f }; // 注視点（回転の中心）
    float targetDistance_ = 50.0f;             // 注視点からの距離

public:

    Input* input = Input::GetInstance();

    void Initialize()override;
    void Update()override;

    TransformationMatrix CalculateWVP(const Transform& objectTransform) override;

    Matrix4x4 GetViewMatrix() override { return viewMatrix_; }
    Matrix4x4 GetProjectionMatrix() override { return projectionMatrix_; }

    void DrawFrustum(Camera* normalCamera);
};