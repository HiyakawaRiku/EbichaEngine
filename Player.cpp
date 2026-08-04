#include "Player.h"
#include <cmath>


void Player::Initialize()
{
	transformBase_.Initialize();
	transformBase_.translate = { 0, 0, 0 };

	modelBody_ = new Model();
	modelBody_->Initialize("player.obj");
	modelBody_->transform.translate = { 0.0f, 0.0f, 0.0f };
	modelBody_->transform.parent = &transformBase_;

	modelHead_ = new Model();
	modelHead_->Initialize("float_Head.obj");
	modelHead_->transform.translate = { 0.0f, 1.5f, 0.0f };
	modelHead_->transform.parent = &transformBase_;

	modelL_arm_ = new Model();
	modelL_arm_->Initialize("float_L_arm.obj");
	modelL_arm_->transform.translate = { -1.2f, 0.5f, 0.0f };
	modelL_arm_->transform.parent = &transformBase_;

	modelR_arm_ = new Model();
	modelR_arm_->Initialize("float_R_arm.obj");
	modelR_arm_->transform.translate = { 1.2f, 0.5f, 0.0f };
	modelR_arm_->transform.parent = &transformBase_;

	textureHandle_ = TextureManager::GetInstance()->Load("resources/tex.png", DirectXCommon::GetInstance()->GetCommandList());
}

void Player::Update(Camera* activeCamera_)
{
	Vector3 velocity_ = {};

	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)|| Input::GetInstance()->PushKey(DIK_W) || Input::GetInstance()->PushKey(DIK_S)) {
		Vector3 acceleration{};
		if (Input::GetInstance()->PushKey(DIK_D)) {
			acceleration.x += kAcceleration;
		}
		else if (Input::GetInstance()->PushKey(DIK_A)) {
			acceleration.x -= kAcceleration;
		}
		else if (Input::GetInstance()->PushKey(DIK_W)) {
			acceleration.z += kAcceleration;
		}
		else if (Input::GetInstance()->PushKey(DIK_S)) {
			acceleration.z -= kAcceleration;
		}

		velocity_ += acceleration;
	}

	//// 十字キー右（長押し移動など）
	//if (Input::GetInstance()->PushButton(XINPUT_GAMEPAD_DPAD_RIGHT)) {
	//	// MoveRight()
	//}

	// 左スティックの傾きで移動
	float moveX = Input::GetInstance()->GetLeftStickX(); // -1.0 〜 1.0
	float moveZ = Input::GetInstance()->GetLeftStickY();

	// 入力によるローカル移動ベクトル（画面の手前/奥、左右）
	Vector3 localMove = { moveX + velocity_.x, 0.0f, moveZ + velocity_.z };

	// 入力がある場合のみ処理
	if (localMove.x != 0.0f || localMove.z != 0.0f) {
		Vector3 worldMove = localMove;

		// カメラ情報が存在する場合、カメラのY軸回転に合わせて移動ベクトルを回転
		if (activeCamera_) {
			float cameraRotateY = activeCamera_->transform_.rotate.y;
			Matrix4x4 matRotateY = MakeRotateYMatrix(cameraRotateY);
			worldMove = TransformNormal(localMove, matRotateY);
		}

		// 1. 目標角度を算出
		float targetAngle = std::atan2(worldMove.x, worldMove.z);

		// 2. EMath::LerpShortAngle を使って現在角度から目標角度へ滑らかに補間
		const float kRotateSpeed = 0.15f; // 補間率 (0.0f 〜 1.0f)
		modelBody_->transform.rotate.y = EMath::LerpShortAngle(
			modelBody_->transform.rotate.y,
			targetAngle,
			kRotateSpeed
		);

		// 移動量を位置に加算
		modelBody_->transform.translate += worldMove;
	}

	modelBody_->transform.UpdateMatrix();

	modelBody_->Update(activeCamera_);
	modelHead_->Update(activeCamera_);
	modelL_arm_->Update(activeCamera_);
	modelR_arm_->Update(activeCamera_);
}

void Player::Draw()
{
	modelBody_->Draw(textureHandle_);
	modelHead_->Draw(textureHandle_);
	modelL_arm_->Draw(textureHandle_);
	modelR_arm_->Draw(textureHandle_);
}
