#include "Mesh.h"

void Mesh::Initialize()
{
	CreateVertexResource();
	CreateVertexData();
	CreateMaterialResource();
	CreateWvpResource();
}

void Mesh::Draw()
{
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);    // VBVを設定
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//マテリアルCBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// WVP用CBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, dxCommon_->textureSrvHandleGPU);
	// 描画！（DrawCall/ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
	dxCommon_->GetCommandList()->DrawInstanced(6, 1, 0, 0);
}

void Mesh::CreateVertexResource()
{
	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 6);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Mesh::CreateVertexData()
{
	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr,
		reinterpret_cast<void**>(&vertexData));
	// 左下
	vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 1.0f };
	// 上
	vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 0.5f, 0.0f };
	// 右下
	vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	// 左下
	vertexData[3].position = { -0.5f, -0.5f, 0.5f, 1.0f };
	vertexData[3].texcoord = { 0.0f, 1.0f };
	// 上
	vertexData[4].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[4].texcoord = { 0.5f, 0.0f };
	// 右下
	vertexData[5].position = { 0.5f, -0.5f, -0.5f, 1.0f };
	vertexData[5].texcoord = { 1.0f, 1.0f };
}

void Mesh::CreateMaterialResource()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Vector4));
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	*materialData = { 1.0f,1.0f,1.0f,1.0f };
}

void Mesh::CreateWvpResource()
{
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Matrix4x4));
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	*wvpData = MakeIdentity4x4();
}