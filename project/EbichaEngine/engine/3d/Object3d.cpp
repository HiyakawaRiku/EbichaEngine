#include "Object3d.h"
#include "Object3dCommon.h"

#include "TextureManager.h"
//ファイルに書いたり読んだりするライブラリ
#include <fstream>
//時間を扱うライブラリ
#include <chrono>

#include "ModelManager.h"
#include "Camera.h"


void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	this->object3dCommon = object3dCommon;
	this->camera_ = object3dCommon->GetDefaultCamera();


	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource = CreateBufferResource(object3dCommon->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書きこんでおく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	directionalLightResource = CreateBufferResource(object3dCommon->GetDxCommon()->GetDevice(), sizeof(DirectionalLight));
	// データを書き込む
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 単位行列を書きこんでおく
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;


	//Transform関数を作る
	transform = { { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };
	cameraTransform = { { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,-10.0f } };
}

void Object3d::Update()
{

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;
	
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else {
		worldViewProjectionMatrix = worldMatrix;
	}
	
	wvpData->World = worldMatrix;
	wvpData->WVP = worldViewProjectionMatrix;

}

void Object3d::Draw()
{

	// wvp用のCBufferの場所を設定
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// directionalLight用のCBufferの場所を設定
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	if(model)model->Draw();
}

void Object3d::Finalize()
{

}

void Object3d::SetModel(const std::string& filePath)
{
	model = ModelManager::GetInstance()->FindModel(filePath);
}


