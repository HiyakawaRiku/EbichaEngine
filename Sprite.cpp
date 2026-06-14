#include "Sprite.h"

void Sprite::Initialize()
{
	CreateVertexResource();
	CreateVertexData();
	CreateIndexResource();
	CreateIndexData();
	CreateMaterialResource();
	CreateTransformationMatrix();
	CreateDirectionalLight();
}

void Sprite::Draw(UINT vertexCountPerInstance)
{
	// Spriteの描画。変更が必要なものだけ変更する
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);    // VBVを設定
	dxCommon_->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//マテリアルCBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformationMatrixCBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, dxCommon_->textureSrvHandleGPU[Texture::monsterBall]);
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 描画！（DrawCall/ドローコール）
	dxCommon_->GetCommandList()->DrawIndexedInstanced(vertexCountPerInstance, 1, 0, 0, 0);
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
	// 1枚目の三角形
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

void Sprite::CreateMaterialResource()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = { 1.0f,1.0f,1.0f,1.0f };
}

void Sprite::CreateTransformationMatrix()
{
	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書きこんでおく
	transformationMatrixData->WVP = MakeIdentity4x4();

}

void Sprite::CreateDirectionalLight()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	directionalLightResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(DirectionalLight));
	// マテリアルにデータを書き込む
	DirectionalLight* directionalLightData = nullptr;
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 今回は赤を書き込んでみる
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
}
