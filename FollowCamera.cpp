#include "FollowCamera.h"
#include "Player.h"

void FollowCamera::Initialize()
{
	viewProjection_.Initialize();
}

void FollowCamera::Update()
{
	// 追従対象がいれば
	if (target_) {
		// 追従対象からカメラまでのオフセット
		Vector3 offset = { 0.0f, 2.0f, -10.0f };

		Matrix4x4 matRotate = MakeIdentity4x4(); // もしくは適切な初期化
		matRotate = MakeRotateYMatrix(viewProjection_.transform_.rotate.y);

		//オフセットをカメラの回転に合わせて回転させる
		offset = TransformNormal(offset, matRotate);

		//座標をコピーしてオフセット分ずらす
		viewProjection_.transform_.translate = target_->translate + offset;
	}

	//if (Input::GetInstance()->PushKey(DIK_D) || Input::GetInstance()->PushKey(DIK_A) || Input::GetInstance()->PushKey(DIK_W) || Input::GetInstance()->PushKey(DIK_S)) {
	//	Vector3 acceleration{};
	//	if (Input::GetInstance()->PushKey(DIK_D)) {
	//		acceleration.x += kAcceleration;
	//	}
	//	else if (Input::GetInstance()->PushKey(DIK_A)) {
	//		acceleration.x -= kAcceleration;
	//	}
	//	else if (Input::GetInstance()->PushKey(DIK_W)) {
	//		acceleration.z += kAcceleration;
	//	}
	//	else if (Input::GetInstance()->PushKey(DIK_S)) {
	//		acceleration.z -= kAcceleration;
	//	}

	//	velocity_ += acceleration;
	//}

	//ビュー行列の更新
	viewProjection_.Update();
}
