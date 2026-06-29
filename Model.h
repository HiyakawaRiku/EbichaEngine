#pragma once
#include "DirectXCommon.h"
#include "BaseObject.h"

class Model
{
public:
	void Initialize();
	void Draw();

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// データを書き込む
	TransformationMatrix* wvpData = nullptr;
	// マテリアルにデータを書き込む
	DirectionalLight* directionalLightData = nullptr;
private:

	// モデル読み込み
	ModelData modelData = LoadObjFile("resources", "plane.obj");

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();

private:
	void CreateModelSphere();
	void CreateMaterialResource();
	void CreateWvpResource();
	void CreateDirectionalLight();
};

