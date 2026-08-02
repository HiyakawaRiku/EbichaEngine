#include "Player.h"

inline Vector3& operator+=(Vector3& lhs, const Vector3& rhs) {
	lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z;
	return lhs;
}

inline Vector3& operator-=(Vector3& lhs, const Vector3& rhs) {
	lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z;
	return lhs;
}

void Player::Initialize()
{
	modelTeapot_ = new Model();
	modelTeapot_->Initialize("resources", "teapot.obj");
}

void Player::Update(Camera* activeCamera_)
{
	Vector3 velocity_ = {};

	if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A)) {
		Vector3 acceleration{};
		if (Input::GetInstance()->PushKey(DIK_D)) {
			acceleration.x += kAcceleration;
		}
		else if (Input::GetInstance()->PushKey(DIK_A)) {
			acceleration.x -= kAcceleration;
		}

		velocity_ += acceleration;
	}

	modelTeapot_->transform.translate += velocity_;

	modelTeapot_->Update(activeCamera_);
}

void Player::Draw()
{
	modelTeapot_->Draw(1);
}
