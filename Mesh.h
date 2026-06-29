#pragma once
#include "DirectXCommon.h"
#include "BaseObject.h"

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
	// マテリアルにデータを書き込む
	DirectionalLight* directionalLightData = nullptr;


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