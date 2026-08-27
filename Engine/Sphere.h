#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "EMath.h"
#include "Transform.h"
#include "Camera.h"

class Sphere {
public:
	Sphere() = default;
	~Sphere() = default;

	void Initialize();
	void Update(Camera* camera);
	void Draw(TextureHandle textureHandle);

public:
	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform uvTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	uint32_t lightingType = 1;

	uint32_t kSubdivision = 16;

	// ★追加: 外部制御用ゲッター
	PointLight* GetPointLightData() { return pointLightData_; }
	SpotLight* GetSpotLightData() { return spotLightData_; }

private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateIndexResource();
	void CreateIndexData();

	void CreateMaterialResource();
	void CreateWvpResource();
	void CreateDirectionalLight();
	// ★追加: リソース生成関数
	void CreatePointLight();
	void CreateSpotLight();

private:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	Camera* camera_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	uint32_t vertexCount_ = 0;

	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	uint32_t indexCount_ = 0;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	// ★追加: ライト用定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_;

	// バッファマップ用ポインタ
	Material* materialData_ = nullptr;
	TransformationMatrix* wvpData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	// ★追加: ライト用構造体ポインタ
	PointLight* pointLightData_ = nullptr;
	SpotLight* spotLightData_ = nullptr;
};