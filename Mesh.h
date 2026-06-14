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
private:
	DirectXCommon* dxCommon_=DirectXCommon::GetInstance();
public:
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	// データを書き込む
	Matrix4x4* wvpData = nullptr;
private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateMaterialResource();
	void CreateWvpResource();
};