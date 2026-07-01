#pragma once
#include "BaseObject.h"

class Sphere:public BaseObject
{
public:
	void Initialize(uint32_t kSubdivision);
	void Draw(UINT vertexCountPerInstance, bool useMonsterBall);
private:
	void CreateVertexResource(uint32_t kSubdivision);
	void CreateVertexData(uint32_t kSubdivision);

	// 頂点バッファビューを作成する
//D3D12_INDEX_BUFFER_VIEW indexBufferView{};
//Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
//void CreateIndexResource();
	//void CreateIndexData();

};

