#pragma once
#include "Struct.h"

class Camera
{
public:
    // 任意のTransformから、このカメラ基準のWVP行列を計算して返す
    virtual TransformationMatrix CalculateWVP(const Transform& objectTransform);
    virtual TransformationMatrix CalculateWVP2D(const Transform& objectTransform);

    virtual void Initialize() { CreateCameraResource(); }
    virtual void Update() {}

    Transform transform_{ {1.0f,1.0f,1.0f},{0.1f,0.0f,0.0f},{0.0f,5.0f,-20.0f} };

    virtual Matrix4x4 GetViewMatrix() {
        Matrix4x4 cameraMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
        return Inverse(cameraMatrix);
    }

    virtual Matrix4x4 GetProjectionMatrix() {
        WinApp* app = WinApp::GetInstance();
        return MakePerspectiveFovMatrix(0.45f, float(app->kWindowWidth) / float(app->kWindowHeight), 0.1f, 100.0f);
    }

    // ★追加: ConstantBufferリソースを取得するゲッター
    ID3D12Resource* GetCameraResource() const { return cameraResource_.Get(); }

private:
    void CreateCameraResource();

protected:
    WinApp* app_ = WinApp::GetInstance();

    // ★追加: 定数バッファリソースとマップ先ポインタ
    Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
    CameraForGPU* cameraData_ = nullptr;
};