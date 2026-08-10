#include "Camera.h"

// 3Dオブジェクト用のWVP行列を計算して返す
TransformationMatrix Camera::CalculateWVP(const Transform& objectTransform)
{
	// 1. オブジェクトの Transform をコピーして行列を最新に更新する
	Transform currentTransform = objectTransform;
	currentTransform.UpdateMatrix();

	Matrix4x4 worldMatrix = currentTransform.matWorld;

	// 2. カメラ自身の状態からビュー行列を計算
	Matrix4x4 cameraMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);

	// 3. 射影行列を計算
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(app_->kWindowWidth) / float(app_->kWindowHeight), 0.1f, 100.0f);

	// 4. 行列を合成 (World * View * Projection)
	Matrix4x4 worldViewMatrix = Multiply(worldMatrix, viewMatrix);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldViewMatrix, projectionMatrix);

	// 構造体にまとめて返す
	TransformationMatrix result = { worldViewProjectionMatrix, worldMatrix };
	return result;
}

// 2D（スプライトなど）オブジェクト用のWVP行列を計算して返す
TransformationMatrix Camera::CalculateWVP2D(const Transform& objectTransform)
{
	// 2Dオブジェクトのワールド行列を計算
	Matrix4x4 worldMatrixSprite = objectTransform.matWorld;

	// 2Dはカメラの移動や回転を無視するため、ビュー行列は単位行列
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();

	// 平行投影（スクリーン座標系）の射影行列を計算
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(app_->kWindowWidth), float(app_->kWindowHeight), 0.0f, 100.0f);

	// すべてを掛け合わせてWVP行列を合成
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));

	TransformationMatrix result = { worldViewProjectionMatrixSprite, worldMatrixSprite };
	return result;
}

void Camera::CreateCameraResource() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	cameraResource_ = DirectXUtils::CreateBufferResource(dxCommon->GetDevice(), sizeof(CameraForGPU));
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	cameraData_->worldPosition = transform_.translate;
}
