#pragma once
#include "BaseObject.h"

class Sprite:public BaseObject{
public:
	void Initialize()override;
	void Draw(UINT vertexCountPerInstance);
private:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
public:
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;

private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateIndexResource();
	void CreateIndexData();
};