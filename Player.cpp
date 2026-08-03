#include "Player.h"



void Player::Initialize()
{
	model_ = new Model();
	model_->Initialize("resources", "player.obj");
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

	model_->transform.translate += velocity_;

	model_->Update(activeCamera_);
}

void Player::Draw()
{
	model_->Draw(6);
}
