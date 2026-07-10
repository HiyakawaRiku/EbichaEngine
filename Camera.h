#pragma once
#include "BaseObject.h"

class Camera
{
public:
    // 任意のTransformから、このカメラ基準のWVP行列を計算して返す
    TransformationMatrix CalculateWVP(const Transform& objectTransform);
    TransformationMatrix CalculateWVP2D(const Transform& objectTransform);

    // main.cppで生で触っていたメンバをプロパティ（Get/Set）にするか、
    // ImGui用にそのままにする場合でも、カメラが「状態」を持つことを意識する
    Transform transform_{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

private:
    WinApp* app_ = WinApp::GetInstance();
};