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
	Vector3 normal;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

enum Texture {
	uvChecker,
	monsterBall
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
	//D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere;

	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// データを書き込む
	TransformationMatrix* wvpData = nullptr;


private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateVertexResourceSphere(uint32_t kSubdivision);
	void CreateVertexDataSphere(uint32_t kSubdivision);
	//void CreateIndexResource();
	//void CreateIndexData();
	void CreateMaterialResource();
	void CreateWvpResource();
	void CreateDirectionalLight();
};