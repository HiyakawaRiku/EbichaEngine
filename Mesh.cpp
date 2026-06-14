#include "Mesh.h"

void Mesh::Initialize()
{
	CreateVertexResource();
	CreateVertexData();
	CreateMaterialResource();
	CreateWvpResource();
}

void Mesh::InitializeSphere(uint32_t kSubdivision)
{
	CreateVertexResourceSphere(kSubdivision);
	CreateVertexDataSphere(kSubdivision);
	CreateMaterialResource();
	CreateWvpResource();
}

void Mesh::Draw(UINT vertexCountPerInstance)
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
	dxCommon_->GetCommandList()->DrawInstanced(vertexCountPerInstance, 1, 0, 0);
}

void Mesh::DrawSphere(UINT vertexCountPerInstance)
{
	uint32_t vertexCount = vertexCountPerInstance * vertexCountPerInstance * 6; // 緯度×経度×6頂点（2三角形）

	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);    // VBVを設定
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//マテリアルCBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// WVP用CBufferの場所を設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, dxCommon_->textureSrvHandleGPU);
	// 描画！（DrawCall/ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
	dxCommon_->GetCommandList()->DrawInstanced(vertexCount, 1, 0, 0);
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

void Mesh::CreateVertexResourceSphere(uint32_t kSubdivision)
{

	uint32_t vertexCount = kSubdivision * kSubdivision * 6; // 緯度×経度×6頂点（2三角形）
	// 実際に頂点リソースを作る
	vertexResourceSphere = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * vertexCount;
	// 1頂点あたりのサイズ
	vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);
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

void Mesh::CreateVertexDataSphere(uint32_t kSubdivision)
{
	float pi = 3.14159265359f;

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResourceSphere->Map(0, nullptr,
		reinterpret_cast<void**>(&vertexData));

	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	const float kLatEvery = pi / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;

			float u = float(lonIndex) / float(kSubdivision);
			float v = 1.0f - float(latIndex) / float(kSubdivision);
			float uNext = float(lonIndex + 1) / float(kSubdivision);
			float vNext = 1.0f - float(latIndex + 1) / float(kSubdivision);

			// 各頂点の位置を計算するヘルパー
			auto calcPos = [&](float latVal, float lonVal) -> Vector4 {
				return Vector4(
					cosf(latVal) * cosf(lonVal),
					sinf(latVal),
					cosf(latVal) * sinf(lonVal),
					1.0f
				);
			};

			// 三角形1（左上から右下への対角線で分割）
			// 0: 左上 (lat, lon)
			vertexData[start + 0].position = calcPos(lat, lon);
			vertexData[start + 0].texcoord = { u, v };
			// 1: 左下 (lat+1, lon)
			vertexData[start + 1].position = calcPos(lat + kLatEvery, lon);
			vertexData[start + 1].texcoord = { u, vNext };
			// 2: 右下 (lat+1, lon+1)
			vertexData[start + 2].position = calcPos(lat + kLatEvery, lon + kLonEvery);
			vertexData[start + 2].texcoord = { uNext, vNext };

			// 三角形2
			// 3: 左上 (lat, lon)
			vertexData[start + 3].position = calcPos(lat, lon);
			vertexData[start + 3].texcoord = { u, v };
			// 4: 右下 (lat+1, lon+1)
			vertexData[start + 4].position = calcPos(lat + kLatEvery, lon + kLonEvery);
			vertexData[start + 4].texcoord = { uNext, vNext };
			// 5: 右上 (lat, lon+1)
			vertexData[start + 5].position = calcPos(lat, lon + kLonEvery);
			vertexData[start + 5].texcoord = { uNext, v };
		}
	}
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