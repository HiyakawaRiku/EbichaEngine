#pragma once
#include "DirectXCommon.h"
#include "Matrix.h"
#include "Mesh.h"

class Sprite{
public:
	void Initialize();
	void Draw(UINT vertexCountPerInstance);
private:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
public:
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// データを書き込む
	TransformationMatrix* transformationMatrixData = nullptr;

private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateIndexResource();
	void CreateIndexData();
	void CreateMaterialResource();
	void CreateTransformationMatrix();
	void CreateDirectionalLight();
};