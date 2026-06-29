#include "BaseObject.h"
#include "BaseObject.h"
#include "BaseObject.h"
#include "BaseObject.h"
#include "BaseObject.h"
#include "BaseObject.h"

void BaseObject::Initialize(){

	CreateVertexResource();
	CreateVertexData();
	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
}
void BaseObject::Draw(){}

void BaseObject::CreateVertexResource()
{
}

void BaseObject::CreateVertexData()
{
}

void BaseObject::CreateMaterialResource()
{
}

void BaseObject::CreateWvpResource()
{
}

void BaseObject::CreateDirectionalLight()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	directionalLightResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 今回は赤を書き込んでみる
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
}
