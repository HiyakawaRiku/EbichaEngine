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
	// トランスフォームとカラー設定
	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform uvTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	uint32_t lightingType = 1; // 3DなのでデフォルトでLambert(1)等を有効化

	uint32_t kSubdivision = 16;

private:
	void CreateVertexResource();
	void CreateVertexData();
	void CreateIndexResource();
	void CreateIndexData();

	void CreateMaterialResource();
	void CreateWvpResource();
	void CreateDirectionalLight();

private:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	Camera* camera_ = nullptr;

	// 頂点・インデックスバッファ
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

	// バッファマップ用ポインタ
	Material* materialData_ = nullptr;
	TransformationMatrix* wvpData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
};