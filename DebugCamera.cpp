#include "DebugCamera.h"
#include "Input.h"

Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b) {
    return Multiply(a, b);
}

void DebugCamera::Initialize() {
    translation_ = { 0.0f, 0.0f, -50.0f };
    matRot_ = MakeIdentity4x4();
}

void DebugCamera::Update() {
    const float kMoveSpeed = 0.5f;
    const float kRotateSpeed = 0.02f;

    // === 回転処理（累積方式）===
    // 追加回転分の回転行列を生成
    Matrix4x4 matRotDelta = MakeIdentity4x4();

    // X軸回りの回転入力
    if (PushKey(DIK_UP)) {
        matRotDelta = MakeRotateXMatrix(-kRotateSpeed) * matRotDelta;
    }
    if (PushKey(DIK_DOWN)) {
        matRotDelta = MakeRotateXMatrix(kRotateSpeed) * matRotDelta;
    }

    // Y軸回りの回転入力
    if (PushKey(DIK_RIGHT)) {
        matRotDelta = MakeRotateYMatrix(kRotateSpeed) * matRotDelta;
    }
    if (PushKey(DIK_LEFT)) {
        matRotDelta = MakeRotateYMatrix(-kRotateSpeed) * matRotDelta;
    }

    // Z軸回りの回転入力
    if (PushKey(DIK_Q)) {
        matRotDelta = MakeRotateZMatrix(kRotateSpeed) * matRotDelta;
    }
    if (PushKey(DIK_E)) {
        matRotDelta = MakeRotateZMatrix(-kRotateSpeed) * matRotDelta;
    }

    // 累積の回転行列を合成
    matRot_ = matRotDelta * matRot_;

    // === 移動処理 ===
    // 前後移動
    if (PushKey(DIK_W)) {
        Vector3 move = { 0.0f, 0.0f, kMoveSpeed };
        move = Transforms(move, matRot_);  // 累積回転行列で回転
        translation_.x += move.x;
        translation_.y += move.y;
        translation_.z += move.z;
    }
    if (PushKey(DIK_S)) {
        Vector3 move = { 0.0f, 0.0f, -kMoveSpeed };
        move = Transforms(move, matRot_);
        translation_.x += move.x;
        translation_.y += move.y;
        translation_.z += move.z;
    }

    // 左右移動
    if (PushKey(DIK_D)) {
        Vector3 move = { kMoveSpeed, 0.0f, 0.0f };
        move = Transforms(move, matRot_);
        translation_.x += move.x;
        translation_.y += move.y;
        translation_.z += move.z;
    }
    if (PushKey(DIK_A)) {
        Vector3 move = { -kMoveSpeed, 0.0f, 0.0f };
        move = Transforms(move, matRot_);
        translation_.x += move.x;
        translation_.y += move.y;
        translation_.z += move.z;
    }

    // 上下移動
    if (PushKey(DIK_SPACE)) {
        Vector3 move = { 0.0f, kMoveSpeed, 0.0f };
        move = Transforms(move, matRot_);
        translation_.x += move.x;
        translation_.y += move.y;
        translation_.z += move.z;
    }
    if (PushKey(DIK_LSHIFT)) {
        Vector3 move = { 0.0f, -kMoveSpeed, 0.0f };
        move = Transforms(move, matRot_);
        translation_.x += move.x;
        translation_.y += move.y;
        translation_.z += move.z;
    }

    // === ビュー行列の更新 ===
    // 座標から平行移動行列を計算する
    Matrix4x4 translationMatrix = MakeTranslateMatrix(translation_);

    // 累積回転行列と平行移動行列からワールド行列を計算する
    Matrix4x4 cameraWorldMatrix = matRot_ * translationMatrix;

    // ワールド行列の逆行列をビュー行列に代入する
    viewMatrix_ = Inverse(cameraWorldMatrix);

    // === 射影行列の計算 ===
    projectionMatrix_ = MakePerspectiveFovMatrix(
        0.45f,
        1280.0f / 720.0f,
        0.1f,
        1000.0f
    );
}