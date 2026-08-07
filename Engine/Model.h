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

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// インスタンスごとに送信するデータ
struct ParticleInstanceData {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

class Model {
public:
	// ★追加: Playerなどで階層構造（親子関係）をつくるための Transform★
	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// 色とライト設定
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	int32_t lightingType = 1; // LightType_Lambert

	// 初期化
	void Initialize(const std::string& filename);

	// ★追加: 自身の transform を使って描画する関数
	void Draw(Camera* camera, TextureHandle textureHandle);

	// 外部から任意の transform を渡して描画する関数
	void Draw(const Transform& transform, Camera* camera, TextureHandle textureHandle);

	// インスタンス数を指定して描画する関数を追加
	void DrawInstanced(const std::vector<Particle>& transforms, Camera* camera, TextureHandle textureHandle);

private:
	void CreateModelSphere();
	void CreateMaterialResource();
	void CreateWvpResource();
	void CreateDirectionalLight();
	void CreateInstanceResource();

private:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();

	ModelData modelData_;

	// 頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	uint32_t vertexCount_ = 0;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;

	// バッファのマップ先ポインタ
	Material* materialData_ = nullptr;
	TransformationMatrix* wvpData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	int instanceCount_ = 3;

private:
	static const uint32_t kMaxInstanceCount = 1000; // 最大インスタンス数

	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
	TransformationMatrix* instanceData_ = nullptr;
	uint32_t instanceSrvIndex_ = 100;

	GraphicsPipelineManager graphicsPipelineManager_;

};

// ヘルパー関数
MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);