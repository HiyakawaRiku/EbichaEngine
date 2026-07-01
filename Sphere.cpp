#include "Sphere.h"

void Sphere::Initialize(uint32_t kSubdivision)
{
	BaseObject::Initialize();
	CreateVertexResource(kSubdivision);
	CreateVertexData(kSubdivision);
}

void Sphere::Draw(UINT vertexCountPerInstance, bool useMonsterBall)
{
	uint32_t vertexCount = vertexCountPerInstance * vertexCountPerInstance * 6;

	BaseObject::Draw(vertexBufferView, materialResource, wvpResource, directionalLightResource, vertexCount);
}

void Sphere::CreateVertexResource(uint32_t kSubdivision)
{
	uint32_t vertexCount = kSubdivision * kSubdivision * 6; // 緯度×経度×6頂点（2三角形）
	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Sphere::CreateVertexData(uint32_t kSubdivision)
{
	float pi = 3.14159265359f;

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr,
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
			vertexData[start + 0].normal.x = vertexData[start + 0].position.x;
			vertexData[start + 0].normal.y = vertexData[start + 0].position.y;
			vertexData[start + 0].normal.z = vertexData[start + 0].position.z;
			// 1: 左下 (lat+1, lon)
			vertexData[start + 1].position = calcPos(lat + kLatEvery, lon);
			vertexData[start + 1].texcoord = { u, vNext };
			vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
			vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
			vertexData[start + 1].normal.z = vertexData[start + 1].position.z;

			// 2: 右下 (lat+1, lon+1)
			vertexData[start + 2].position = calcPos(lat + kLatEvery, lon + kLonEvery);
			vertexData[start + 2].texcoord = { uNext, vNext };
			vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
			vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
			vertexData[start + 2].normal.z = vertexData[start + 2].position.z;

			// 三角形2
			// 3: 左上 (lat, lon)
			vertexData[start + 3].position = calcPos(lat, lon);
			vertexData[start + 3].texcoord = { u, v };
			vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
			vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
			vertexData[start + 3].normal.z = vertexData[start + 3].position.z;

			// 4: 右下 (lat+1, lon+1)
			vertexData[start + 4].position = calcPos(lat + kLatEvery, lon + kLonEvery);
			vertexData[start + 4].texcoord = { uNext, vNext };
			vertexData[start + 4].normal.x = vertexData[start + 4].position.x;
			vertexData[start + 4].normal.y = vertexData[start + 4].position.y;
			vertexData[start + 4].normal.z = vertexData[start + 4].position.z;

			// 5: 右上 (lat, lon+1)
			vertexData[start + 5].position = calcPos(lat, lon + kLonEvery);
			vertexData[start + 5].texcoord = { uNext, v };
			vertexData[start + 5].normal.x = vertexData[start + 5].position.x;
			vertexData[start + 5].normal.y = vertexData[start + 5].position.y;
			vertexData[start + 5].normal.z = vertexData[start + 5].position.z;

		}
	}
}


//void Mesh::CreateIndexResource()
//{
//	// Sprite用の頂点リソースを作る
//	indexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * 6);
//
//
//	// リソースの先頭のアドレスから使う
//	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
//	// 使用するリソースのサイズは頂点6つ分のサイズ
//	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
//	// 1頂点あたりのサイズ
//	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
//}
//void Mesh::CreateIndexData()
//{
//	uint32_t* indexData = nullptr;
//	indexResource->Map(0, nullptr,
//		reinterpret_cast<void**>(&indexData));
//	// 1枚目の三角形
//	indexData[0] = 0;
//	indexData[1] = 1;
//	indexData[2] = 2;
//	indexData[3] = 1;
//	indexData[4] = 3;
//	indexData[5] = 2;
//}