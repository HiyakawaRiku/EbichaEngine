#include "Triangle.h"
#include "Model.h"

void Triangle::Initialize(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2)
{
	BaseObject::Initialize();

	CreateVertexResource();
	CreateVertexData(vertex0,vertex1,vertex2);
}

void Triangle::Draw(UINT vertexCountPerInstance)
{
	BaseObject::Draw(vertexBufferView, materialResource, wvpResource, directionalLightResource, vertexCountPerInstance);
}

void Triangle::CreateVertexResource()
{
	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 3);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 3;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Triangle::CreateVertexData(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2)
{
	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr,
		reinterpret_cast<void**>(&vertexData));
	// 左下
	vertexData[0].position = vertex0;
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f, 0.0f,-1.0f };
	// 上
	vertexData[1].position = vertex1;
	vertexData[1].texcoord = { 0.5f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f,-1.0f };
	// 右下
	vertexData[2].position = vertex2;
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f,-1.0f };
}