#include "Mesh.h"

void Mesh::Initialize(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2)
{
	CreateVertexResource();
	CreateVertexData(vertex0, vertex1, vertex2);

	// 定数バッファの生成
	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
}

void Mesh::CreateVertexResource()
{
	vertexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 3);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 3;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Mesh::CreateVertexData(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2)
{
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 左下
	vertexData[0].position = vertex0;
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	// 上
	vertexData[1].position = vertex1;
	vertexData[1].texcoord = { 0.5f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertexData[2].position = vertex2;
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
}

void Mesh::CreateMaterialResource()
{
	materialResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->lightingType = 1; // LightType_Lambert
	materialData_->uvTransform = MakeIdentity4x4();
}

void Mesh::CreateWvpResource()
{
	wvpResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = MakeIdentity4x4();
}

void Mesh::CreateDirectionalLight()
{
	directionalLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
}

void Mesh::Draw(const Transform& transform, Camera* camera, TextureHandle textureHandle)
{
	// 1. 行列とマテリアルの更新
	Transform currentTransform = transform;
	currentTransform.UpdateMatrix();

	if (wvpData_ && camera) {
		*wvpData_ = camera->CalculateWVP(currentTransform);
	}

	if (materialData_) {
		materialData_->color = this->color;
		materialData_->lightingType = this->lightingType;
	}

	// 2. 描画コマンドの発行
	auto commandList = dxCommon_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle);
	commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	// 三角形（3頂点）の描画
	commandList->DrawInstanced(3, 1, 0, 0);
}