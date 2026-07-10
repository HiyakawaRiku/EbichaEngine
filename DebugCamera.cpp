#include "DebugCamera.h"
#include "Input.h"
#include "DebugRenderer.h"

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

	if (!ImGui::GetIO().WantCaptureMouse) {
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
	}

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

	Vector3 forward = Transforms({ 0.0f, 0.0f, 1.0f }, matRot_);

	translation_.x = targetPos_.x - forward.x * targetDistance_;
	translation_.y = targetPos_.y - forward.y * targetDistance_;
	translation_.z = targetPos_.z - forward.z * targetDistance_;
	// === ビュー行列の更新 ===
	// 座標から平行移動行列を計算する
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);
	viewMatrix_ = Inverse(matRot_ * matTrans);

	// 射影行列の計算（既存のまま）
	WinApp* app = WinApp::GetInstance();
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(app->kWindowWidth) / float(app->kWindowHeight), 0.1f, 100.0f);
}
void DebugCamera::DrawFrustum(Camera* normalCamera) {
	if (!normalCamera) return;
	// 通常カメラの現在の座標を取得
	Vector3 camPos = normalCamera->transform_.translate;
	Vector3 camRot = normalCamera->transform_.rotate;

	// --- 1. 最もシンプルな「カメラ位置に赤・緑・青の十字線を引く」 ---
	float axisLength = 5.0f; // 線の長さ

	// カメラのローカル方向（前・上・右）のベクトルを計算
	Matrix4x4 camRotMat = MakeRotateMatrix(camRot);
	Vector3 forward = Transforms({ 0.0f, 0.0f, 1.0f }, camRotMat);
	Vector3 up = Transforms({ 0.0f, 1.0f, 0.0f }, camRotMat);
	Vector3 right = Transforms({ 1.0f, 0.0f, 0.0f }, camRotMat);

	// X軸（右方向）：赤
	DebugRenderer::AddLine(camPos, { camPos.x + right.x * axisLength, camPos.y + right.y * axisLength, camPos.z + right.z * axisLength }, { 1.0f, 0.0f, 0.0f, 1.0f });
	// Y軸（上方向）：緑
	DebugRenderer::AddLine(camPos, { camPos.x + up.x * axisLength,    camPos.y + up.y * axisLength,    camPos.z + up.z * axisLength }, { 0.0f, 1.0f, 0.0f, 1.0f });
	// Z軸（前方向・視線）：青
	DebugRenderer::AddLine(camPos, { camPos.x + forward.x * axisLength, camPos.y + forward.y * axisLength, camPos.z + forward.z * axisLength }, { 0.0f, 0.0f, 1.0f, 1.0f });


	// --- 2. ステップアップ：「カメラが見ている視界の箱（簡易四角錐）」を描画する ---
	// カメラの目の前にある「画面の4隅」のローカル座標
	float fovY = 0.45f; // 実際の画角 (ラジアン)
	float aspect = 1280.0f / 720.0f; // 画面のアスペクト比

	// ガイド線を表示したいお好みの奥行き（例: 20の距離の視界枠を見る場合）
	// ※ 100.0f にするとファークリップ（限界の奥）の枠になります
	float d = 20.0f;

	// 三角関数を使って、奥行き d の位置における「正しい半分の高さ」を逆算
	float h = d * std::tan(fovY / 2.0f);
	// アスペクト比を掛けて「正しい半分の横幅」を計算
	float w = h * aspect;

	Vector3 p0 = Transforms({ -w,  h, d }, camRotMat);
	Vector3 p1 = Transforms({ w,  h, d }, camRotMat);
	Vector3 p2 = Transforms({ w, -h, d }, camRotMat);
	Vector3 p3 = Transforms({ -w, -h, d }, camRotMat);

	// 世界の座標（ワールド座標）に変換
	Vector3 w0 = { camPos.x + p0.x, camPos.y + p0.y, camPos.z + p0.z };
	Vector3 w1 = { camPos.x + p1.x, camPos.y + p1.y, camPos.z + p1.z };
	Vector3 w2 = { camPos.x + p2.x, camPos.y + p2.y, camPos.z + p2.z };
	Vector3 w3 = { camPos.x + p3.x, camPos.y + p3.y, camPos.z + p3.z };

	// カメラ位置から4隅へ黄色い線を伸ばす
	Vector4 yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	DebugRenderer::AddLine(camPos, w0, yellow);
	DebugRenderer::AddLine(camPos, w1, yellow);
	DebugRenderer::AddLine(camPos, w2, yellow);
	DebugRenderer::AddLine(camPos, w3, yellow);

	// 4隅を繋いで四角い枠を作る
	DebugRenderer::AddLine(w0, w1, yellow);
	DebugRenderer::AddLine(w1, w2, yellow);
	DebugRenderer::AddLine(w2, w3, yellow);
	DebugRenderer::AddLine(w3, w0, yellow);

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