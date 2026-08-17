#include "FollowCamera.h"
#include "Enemy.h"

void FollowCamera::Initialize()
{
	viewProjection_.Initialize();
}

void FollowCamera::Update()
{
	// 左スティックの傾きで移動
	float moveX = Input::GetInstance()->GetRightStickX(); // -1.0 〜 1.0
	float moveZ = Input::GetInstance()->GetRightStickY();
	viewProjection_.transform_.rotate.y += moveX * kRotateSpeed;

	Vector3 cameraPos = {};

	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT) || Input::GetInstance()->PushKey(DIK_UP) || Input::GetInstance()->PushKey(DIK_DOWN)) {
		Vector3 acceleration{};
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			acceleration.y += kRotateSpeed;
		}
		else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			acceleration.y -= kRotateSpeed;
		}
		else if (Input::GetInstance()->PushKey(DIK_UP)) {
			acceleration.x -= kRotateSpeed;
		}
		else if (Input::GetInstance()->PushKey(DIK_DOWN)) {
			acceleration.x += kRotateSpeed;
		}
		cameraPos += acceleration;
	}
	viewProjection_.transform_.rotate.x += (float)cameraPos.x * kRotateSpeed;
	viewProjection_.transform_.rotate.y += (float)cameraPos.y * kRotateSpeed;
	// 追従対象がいれば
	if (target_) {
		// 追従対象からカメラまでのオフセット
		Vector3 offset = { 0.0f, 6.0f, -30.0f };

		Matrix4x4 matRotate = MakeIdentity4x4(); // もしくは適切な初期化
		matRotate = MakeRotateYMatrix(viewProjection_.transform_.rotate.y);

		//オフセットをカメラの回転に合わせて回転させる
		offset = TransformNormal(offset, matRotate);

		//座標をコピーしてオフセット分ずらす
		viewProjection_.transform_.translate = target_->translate + offset;
	}



	//ビュー行列の更新
	viewProjection_.Update();
}
