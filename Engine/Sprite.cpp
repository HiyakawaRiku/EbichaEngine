#include "Sprite.h"

void Sprite::Initialize()
{
	CreateVertexResource();
	CreateVertexData();
	CreateIndexResource();
	CreateIndexData();

	CreateMaterialResource();
	CreateWvpResource();
	CreateDirectionalLight();
}

void Sprite::CreateVertexResource()
{
	vertexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount_);

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount_;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Sprite::CreateVertexData()
{
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// 単位サイズ（1x1）で作成しておき、transform.scale や size で拡大縮小できるようにする
	vertexData[0].position = { 0.0f, 1.0f, 0.0f, 1.0f }; // 左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f }; // 右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[3].position = { 1.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };
}

void Sprite::CreateIndexResource()
{
	indexResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * indexCount_);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount_;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void Sprite::CreateIndexData()
{
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
}

void Sprite::CreateMaterialResource()
{
	materialResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->lightingType = 0; // None
	materialData_->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateWvpResource()
{
	wvpResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	wvpData_->WVP = MakeIdentity4x4();
}

void Sprite::CreateDirectionalLight()
{
	directionalLightResource_ = DirectXUtils::CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData_->intensity = 1.0f;
}

void Sprite::Update(Camera* camera)
{
	camera_ = camera;

	// 1. 本体の行列計算 (size を反映)
	Transform currentTransform = transform;
	currentTransform.scale.x *= size.x;
	currentTransform.scale.y *= size.y;
	currentTransform.UpdateMatrix();

	if (wvpData_ && camera_) {
		*wvpData_ = camera_->CalculateWVP2D(currentTransform);
	}

	// 2. UV Transform 行列の計算と書き込み
	if (materialData_) {
		materialData_->color = this->color;
		materialData_->lightingType = 0; // ライティング無効

		// ★UV行列の合成計算★
		Matrix4x4 uvMatScale = MakeScaleMatrix(uvTransform.scale);
		Matrix4x4 uvMatRot = MakeRotateZMatrix(uvTransform.rotate.z);
		Matrix4x4 uvMatTrans = MakeTranslateMatrix(uvTransform.translate);

		// Scale * Rotate * Translate
		materialData_->uvTransform = Multiply(uvMatScale, Multiply(uvMatRot, uvMatTrans));
	}
}

void Sprite::Draw(TextureHandle textureHandle)
{
	auto commandList = dxCommon_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle);
	commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	commandList->DrawIndexedInstanced(indexCount_, 1, 0, 0, 0);
}