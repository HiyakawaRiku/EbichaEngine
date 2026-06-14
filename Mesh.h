#pragma once
#include "DirectXCommon.h"
#include "Matrix.h"

struct Vector2 {
	float x, y;
};

struct Vector4 {
	float x, y, z, w;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
};

class Mesh {
public:
	void Initialize();
	void InitializeSphere(uint32_t kSubdivision);
	void Draw(UINT vertexCountPerInstance);
	void DrawSphere(UINT vertexCountPerInstance, bool useMonsterBall);
private:
	DirectXCommon* dxCommon_=DirectXCommon::GetInstance();
public:
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere;

	// マテリアルにデータを書き込む
	Vector4* materialData = nullptr;
	// データを書き込む
	Matrix4x4* wvpData = nullptr;

private:
	void CreateVertexResource();
	void CreateVertexResourceSphere(uint32_t kSubdivision);
	void CreateVertexData();
	void CreateVertexDataSphere(uint32_t kSubdivision);
	void CreateMaterialResource();
	void CreateWvpResource();
};