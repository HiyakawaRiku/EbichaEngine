#pragma once
#include "BaseObject.h"

class Sphere :public BaseObject
{
public:
	void Initialize();
	void Draw(UINT vertexCountPerInstance, bool useMonsterBall);

	uint32_t kSubdivision=16;
private:
	void CreateVertexResource();
	void CreateVertexData();

	// 頂点バッファビューを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	uint32_t indexCount = 0;
	void CreateIndexResource();
	void CreateIndexData();

};

