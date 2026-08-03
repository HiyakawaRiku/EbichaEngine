#include "Player.h"



void Player::Initialize()
{
	model_ = new Model();
	model_->Initialize("head.obj");
	model_->transform.translate.y = 1;
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

	model_->transform.translate.x += moveX;
	model_->transform.translate.z += moveZ;

	model_->transform.translate += velocity_;

	model_->Update(activeCamera_);
}

void Player::Draw()
{
	model_->Draw(6);
}
