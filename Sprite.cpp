#include "Sprite.h"
#include "Camera.h"

void Sprite::Initialize()
{
	BaseObject::Initialize();

	CreateVertexResource();
	CreateVertexData();
	CreateIndexResource();
	CreateIndexData();
}

void Sprite::Update(Camera* camera)
{
	if (wvpData && camera) {
		*wvpData = camera->CalculateWVP2D(transform);
	}

	if (materialData) {
		materialData->color = this->color;
		materialData->lightingType = this->lightingType;
	}
}

void Sprite::CreateVertexResource()
{
	vertexCount = 4;

	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}


void Sprite::CreateVertexData()
{
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr,
		reinterpret_cast<void**>(&vertexData));

	vertexData[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f,-1.0f };
	vertexData[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };   // 左上
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f,-1.0f };
	vertexData[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f,-1.0f };
	vertexData[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };  // 右上
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f, 0.0f,-1.0f };

}

void Sprite::CreateIndexResource()
{
	indexCount = 6;

	// Sprite用の頂点リソースを作る
	indexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * indexCount);

	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * indexCount;
	// 1頂点あたりのサイズ
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}
void Sprite::CreateIndexData()
{
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr,
		reinterpret_cast<void**>(&indexData));
	// 1枚目の三角形
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;


}