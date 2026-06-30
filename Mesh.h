#pragma once
#include "DirectXCommon.h"
#include "BaseObject.h"

class Mesh :public BaseObject{
public:
	void Initialize(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2);
	void InitializeSphere(uint32_t kSubdivision);
	void Draw(UINT vertexCountPerInstance);
	void DrawSphere(UINT vertexCountPerInstance, bool useMonsterBall);
public:
	// 頂点バッファビューを作成する
	//D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere;


private:
	void CreateVertexResource();
	void CreateVertexData(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2);
	void CreateVertexResourceSphere(uint32_t kSubdivision);
	void CreateVertexDataSphere(uint32_t kSubdivision);
	//void CreateIndexResource();
	//void CreateIndexData();
};