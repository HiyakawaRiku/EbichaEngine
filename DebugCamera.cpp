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

    if (ImGui::GetIO().WantCaptureMouse) {
        // 次のフレームで急に視点が跳ねないように、マウスの現在位置だけは毎フレーム記憶させておく
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        prevMousePos_ = currentMousePos;
        return;
    }

    const float kMoveSpeed = 0.5f;
    const float kRotateSpeed = 0.02f;
    const float kMouseRotateSpeed = 0.005f; // 回転の感度
    const float kMouseMoveSpeed = 0.1f;    // 平行移動の感度
    const float kZoomSpeed = 2.0f;         // ズームの感度

    // === 回転処理（累積方式）===
    // 追加回転分の回転行列を生成
    Matrix4x4 matRotDelta = MakeIdentity4x4();

    POINT currentMousePos;
    GetCursorPos(&currentMousePos); // 現在の画面全体でのマウス座標を取得

    if (isFirstFrame_) {
        prevMousePos_ = currentMousePos;
        isFirstFrame_ = false;
    }

    float deltaX = static_cast<float>(currentMousePos.x - prevMousePos_.x);
    float deltaY = static_cast<float>(currentMousePos.y - prevMousePos_.y);

    // ───★ 1. 左クリックドラッグ：注視点を中心に回転 ───
    if ((GetKeyState(VK_LBUTTON) & 0x8000) != 0) {
        if (deltaX != 0.0f) {
            matRotDelta = MakeRotateYMatrix(deltaX * kMouseRotateSpeed) * matRotDelta;
        }
        if (deltaY != 0.0f) {
            matRotDelta = MakeRotateXMatrix(deltaY * kMouseRotateSpeed) * matRotDelta;
        }
    }
    // 累積回転を更新
    matRot_ = matRotDelta * matRot_;

    // ───★ 2. 右クリックドラッグ：カメラの並行移動（注視点も一緒に動かす） ───
    if ((GetKeyState(VK_RBUTTON) & 0x8000) != 0) {
        // カメラのローカル軸（右・上）を取得
        Vector3 right = Transforms({ 1.0f, 0.0f, 0.0f }, matRot_);
        Vector3 up = Transforms({ 0.0f, 1.0f, 0.0f }, matRot_);

        // マウスの移動方向とカメラの移動を合わせる（画面上のドラッグに同期）
        Vector3 move = {
            -(right.x * deltaX - up.x * deltaY) * kMouseMoveSpeed,
            -(right.y * deltaX - up.y * deltaY) * kMouseMoveSpeed,
            -(right.z * deltaX - up.z * deltaY) * kMouseMoveSpeed
        };

        // 注視点を移動させる
        targetPos_.x += move.x;
        targetPos_.y += move.y;
        targetPos_.z += move.z;
    }

    if ((GetKeyState(VK_MBUTTON) & 0x8000) != 0) {
        if (deltaY != 0.0f) {
            // マウスを上に動かしたら近づく（距離マイナス）、下に動かしたら離れる
            targetDistance_ += deltaY * kMouseMoveSpeed * 2.0f;
        }
    }

    if (targetDistance_ < 2.0f) targetDistance_ = 2.0f;

    // 【予備・または併用】もしホイールメッセージが上手く取れない環境の場合、
    // 「キーボードのIとO」でも全く同じように動く予備コードを残しておくと安心です
    if (PushKey(DIK_I)) { targetDistance_ -= kZoomSpeed * 0.2f; }
    if (PushKey(DIK_O)) { targetDistance_ += kZoomSpeed * 0.2f; }

    // 距離がマイナス（注視点を突き抜けて反転する）にならないように最小値をガード（重要）
    if (targetDistance_ < 2.0f) targetDistance_ = 2.0f;

    // 距離がマイナス（突き抜ける）にならないように最小値をガード
    if (targetDistance_ < 1.0f) targetDistance_ = 1.0f;

    // 次のフレームのために位置を保存
    prevMousePos_ = currentMousePos;

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

    Vector3 forward = Transforms({ 0.0f, 0.0f, 1.0f }, matRot_);

    translation_.x = targetPos_.x - forward.x * targetDistance_;
    translation_.y = targetPos_.y - forward.y * targetDistance_;
    translation_.z = targetPos_.z - forward.z * targetDistance_;

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
    Matrix4x4 matTrans = MakeTranslateMatrix(translation_);
    viewMatrix_ = Inverse(matRot_ * matTrans);

    // 射影行列の計算（既存のまま）
    WinApp* app = WinApp::GetInstance();
    projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(app->kWindowWidth) / float(app->kWindowHeight), 0.1f, 100.0f);
}

TransformationMatrix DebugCamera::CalculateWVP(const Transform& objectTransform)
{
    // オブジェクトのワールド行列を計算
    Matrix4x4 worldMatrix = MakeAffineMatrix(objectTransform.scale, objectTransform.rotate, objectTransform.translate);

    // すでに Update() 内で計算済みの viewMatrix_ と projectionMatrix_ を使って合成する！
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix_, projectionMatrix_));

    TransformationMatrix result = { worldViewProjectionMatrix, worldMatrix };
    return result;
}