#pragma once
#include "DirectXCommon.h"
#include "Matrix.h"

#include <fstream>
#include <sstream>

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
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

class BaseObject {
public:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	uint32_t vertexCount = 0;

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	uint32_t indexCount = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	Material* materialData = nullptr;
	TransformationMatrix* wvpData = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	virtual void Initialize();
	virtual void Draw(uint32_t textureIndex);
	virtual void CreateMaterialResource();
	virtual void CreateWvpResource();
	virtual void CreateDirectionalLight();
};