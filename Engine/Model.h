#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "EMath.h"
#include "Transform.h"
#include "Camera.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct MaterialData {
	std::string textureFilePath;
};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
	Node rootNode;
};

// インスタンスごとに送信するデータ
struct ParticleInstanceData {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

class Model {
public:
	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform uvTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// 色とライト設定
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	uint32_t lightingType = 2; // Lambert

	// 初期化
	void Initialize(const std::string& filename);
	void Update(Camera* camera);

	void Draw(Camera* camera, TextureHandle textureHandle);
	void Draw(const Transform& transform, Camera* camera, TextureHandle textureHandle);
	void DrawInstanced(std::list<ParticleData>& particles, Camera* camera, TextureHandle textureHandle);

	// ★追加: ライト制御用ゲッター
	DirectionalLight* GetDirectionalLightData() { return directionalLightData_; }
	PointLight* GetPointLightData() { return pointLightData_; }
	SpotLight* GetSpotLightData() { return spotLightData_; }

private:

	void CreateModelSphere();
	void CreateMaterialResource();
	void CreateWvpResource();
	void CreateDirectionalLight();
	void CreatePointLight();
	void CreateSpotLight();
	void CreateInstanceResource();

private:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	Camera* camera_ = nullptr;

	ModelData modelData_;

	// 頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	uint32_t vertexCount_ = 0;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_;

	// バッファのマップ先ポインタ
	Material* materialData_ = nullptr;
	TransformationMatrix* wvpData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	PointLight* pointLightData_ = nullptr;
	SpotLight* spotLightData_ = nullptr;

	int instanceCount_ = 1;

private:
	static const uint32_t kMaxInstanceCount = 1000;

	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
	ParticleForGPU* instanceData_ = nullptr;
	uint32_t instanceSrvIndex_ = 100;

	GraphicsPipelineManager graphicsPipelineManager_;
};

MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);
Node ReadNode(aiNode* node);