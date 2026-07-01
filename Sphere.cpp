#include "Sphere.h"

void Sphere::Initialize()
{
	BaseObject::Initialize();
	CreateVertexResource();
	CreateVertexData();
	CreateIndexResource();
	CreateIndexData();
}

void Sphere::Draw(UINT vertexCountPerInstance,uint32_t textureIndex)
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
	commandList->SetGraphicsRootDescriptorTable(2, dxCommon_->textureSrvHandleGPU[textureIndex - 1]);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	// 描画！（DrawCall/ドローコール）
	commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void Sphere::CreateVertexResource()
{
	// グリッド状に並べるため、頂点数は (kSubdivision + 1) * (kSubdivision + 1) になる
	uint32_t vertexCount = (kSubdivision + 1) * (kSubdivision + 1);
	// 実際に頂点リソースを作る
	vertexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void Sphere::CreateVertexData()
{
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	uint32_t vertexIndex = 0;
	for (uint32_t lat = 0; lat <= kSubdivision; ++lat) {
		float theta = static_cast<float>(M_PI) * lat / kSubdivision;
		float sinTheta = sinf(theta);
		float cosTheta = cosf(theta);

		for (uint32_t lon = 0; lon <= kSubdivision; ++lon) {
			float phi = 2.0f * static_cast<float>(M_PI) * lon / kSubdivision;
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);

			// 座標 (半径 1.0f の球)
			Vector4 position = {
				sinTheta * cosPhi,
				cosTheta,
				sinTheta * sinPhi,
				1.0f
			};

			// 法線 (中心から外側へ向かうベクトル)
			Vector3 normal = { position.x, position.y, position.z };

			// UV座標
			Vector2 texcoord = {
				static_cast<float>(lon) / kSubdivision,
				static_cast<float>(lat) / kSubdivision
			};

			vertexData[vertexIndex] = { position, texcoord, normal };
			vertexIndex++;
		}
	}
	vertexResource->Unmap(0, nullptr);
}


void Sphere::CreateIndexResource()
{
	// 1つの四角形（1マス）につき2つの三角形（インデックス6個）が必要
	indexCount = kSubdivision * kSubdivision * 6;
	indexResource = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * indexCount);

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * indexCount;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32ビットインデックス
}

void Sphere::CreateIndexData()
{
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	uint32_t indexOffset = 0;
	for (uint32_t lat = 0; lat < kSubdivision; ++lat) {
		for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
			// 現在のグリッドマスを構成する4頂点のインデックスを計算
			// p00 (上左), p01 (上右), p10 (下左), p11 (下右)
			uint32_t p00 = lat * (kSubdivision + 1) + lon;
			uint32_t p01 = p00 + 1;
			uint32_t p10 = (lat + 1) * (kSubdivision + 1) + lon;
			uint32_t p11 = p10 + 1;

			// 三角形1 (上左 -> 上右 -> 下左) の時計回り
			indexData[indexOffset++] = p00;
			indexData[indexOffset++] = p01;
			indexData[indexOffset++] = p10;

			// 三角形2 (上右 -> 下右 -> 下左) の時計回り
			indexData[indexOffset++] = p01;
			indexData[indexOffset++] = p11;
			indexData[indexOffset++] = p10;
		}
	}
	indexResource->Unmap(0, nullptr);
}