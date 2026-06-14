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
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	ID3D12Resource* directionalLightResource;

	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// データを書き込む
	TransformationMatrix* transformationMatrixData = nullptr;

private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateMaterialResource();
	void CreateTransformationMatrix();
	void CreateDirectionalLight();
};