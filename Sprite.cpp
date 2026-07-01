#include "Sprite.h"

void Sprite::Initialize()
{
	BaseObject::Initialize();

	CreateVertexResource();
	CreateVertexData();
	CreateIndexResource();
	CreateIndexData();
}

void Sprite::Draw(UINT vertexCountPerInstance)
{
	auto commandList = dxCommon_->GetCommandList();

	// Spriteの描画。変更が必要なものだけ変更する
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);    // VBVを設定
	commandList->IASetIndexBuffer(&indexBufferView);
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformationMatrixCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	commandList->SetGraphicsRootDescriptorTable(2, dxCommon_->textureSrvHandleGPU[Texture::monsterBall]);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 描画！（DrawCall/ドローコール）
	commandList->DrawIndexedInstanced(vertexCountPerInstance, 1, 0, 0, 0);
}

void Sprite::CreateVertexResource()
{
	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 4);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
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
	// Sprite用の頂点リソースを作る
	indexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * 6);


	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
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