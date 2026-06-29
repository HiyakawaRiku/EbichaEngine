#pragma once
#include "DirectXCommon.h"
#include "BaseObject.h"

class Model:public BaseObject
{
public:
	void Initialize(const std::string& directoryPath, const std::string& filename);
	void Draw();

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere;

private:

	// モデル読み込み
	ModelData modelData;

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();

private:
	void CreateModelSphere();
	void CreateMaterialResource()override;
	void CreateWvpResource()override;
	void CreateDirectionalLight()override;
};

