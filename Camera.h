#pragma once
#include "BaseObject.h"

class Camera
{
public:
    // 任意のTransformから、このカメラ基準のWVP行列を計算して返す
    virtual TransformationMatrix CalculateWVP(const Transform& objectTransform);
    virtual TransformationMatrix CalculateWVP2D(const Transform& objectTransform);

    virtual void Initialize() {}
    virtual void Update() {}

    Transform transform_{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

    virtual Matrix4x4 GetViewMatrix() {
        Matrix4x4 cameraMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
        return Inverse(cameraMatrix);
    }

    virtual Matrix4x4 GetProjectionMatrix() {
        WinApp* app = WinApp::GetInstance();
        return MakePerspectiveFovMatrix(0.45f, float(app->kWindowWidth) / float(app->kWindowHeight), 0.1f, 100.0f);
    }

protected:
    WinApp* app_ = WinApp::GetInstance();
};