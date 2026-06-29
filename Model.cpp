#include "Model.h"

void Model::Initialize(const std::string& directoryPath, const std::string& filename)
{
	modelData = LoadObjFile(directoryPath, filename);

	BaseObject::Initialize();

	CreateModelSphere();
}

void Model::Draw()
{
	BaseObject::Draw(vertexBufferView, materialResource, wvpResource, directionalLightResource, UINT(modelData.vertices.size()));
}

void Model::CreateModelSphere()
{
	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * modelData.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr,
		reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());// 頂点データをリソースにコピー

}