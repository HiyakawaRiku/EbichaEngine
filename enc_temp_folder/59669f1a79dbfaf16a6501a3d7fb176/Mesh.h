#pragma once
#include "DirectXCommon.h"
#include "BaseObject.h"

class Mesh :public BaseObject{
public:
	void Initialize()override;
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

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere;


private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateVertexResourceSphere(uint32_t kSubdivision);
	void CreateVertexDataSphere(uint32_t kSubdivision);
	//void CreateIndexResource();
	//void CreateIndexData();
	void CreateMaterialResource()override;
	void CreateWvpResource()override;
	void CreateDirectionalLight()override;
};