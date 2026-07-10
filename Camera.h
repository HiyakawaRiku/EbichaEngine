#pragma once
#include "BaseObject.h"

class Camera
{
public:
    // 任意のTransformから、このカメラ基準のWVP行列を計算して返す
    virtual TransformationMatrix CalculateWVP(const Transform& objectTransform);
    virtual TransformationMatrix CalculateWVP2D(const Transform& objectTransform);

    virtual void Update() {}

    Transform transform_{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

protected:
    WinApp* app_ = WinApp::GetInstance();
};