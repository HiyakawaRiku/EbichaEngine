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

enum Texture {
	uvChecker,
	monsterBall
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

class BaseObject {
public:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	Material* materialData = nullptr;
	TransformationMatrix* wvpData = nullptr;
	DirectionalLight* directionalLightData = nullptr;

	virtual void Initialize();
	virtual void CreateMaterialResource();
	virtual void CreateWvpResource();
	virtual void CreateDirectionalLight();
};

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);