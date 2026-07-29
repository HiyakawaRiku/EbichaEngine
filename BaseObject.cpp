#include "BaseObject.h"
#include "Camera.h"

void BaseObject::Initialize()
{
	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
}

void BaseObject::Update(Camera* camera)
{
	if (wvpData && camera) {
		*wvpData = camera->CalculateWVP(transform);
	}

	if (materialData) {
		materialData->color = this->color;
		materialData->lightingType = this->lightingType;
	}
}

void BaseObject::Draw(uint32_t textureIndex)
{
	auto commandList = dxCommon_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);    // VBVを設定
	if (indexCount > 0) {
		commandList->IASetIndexBuffer(&indexBufferView);
	}

	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// WVP用CBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	commandList->SetGraphicsRootDescriptorTable(2, dxCommon_->textureSrvHandleGPU[textureIndex-1]);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 描画！（DrawCall/ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
	if (indexCount > 0) {
		commandList->DrawIndexedInstanced(indexCount, instanceCount_, 0, 0, 0);
	}
	else {
		commandList->DrawInstanced(vertexCount, instanceCount_, 0, 0);
	}
}

void BaseObject::CreateMaterialResource()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->lightingType = LightType_Lambert;
	materialData->uvTransform = MakeIdentity4x4();
}

void BaseObject::CreateWvpResource()
{	
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();
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