#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "EMath.h"
#include "Transform.h"
#include "Camera.h"

class Sprite {
public:
	// 本体とUVの Transform
	Transform transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform uvTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// 色とサイズ
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 size = { 100.0f, 100.0f };

	void Initialize();
	void Update(Camera* camera);
	void Draw(TextureHandle textureHandle);

	// ★エラー回避用: materialData_ へのアクセサ（必要に応じて外部から直接触れるようにする）
	Material* GetMaterialData() const { return materialData_; }

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
	uint32_t vertexCount_ = 4;

	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	uint32_t indexCount_ = 6;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;

	// バッファマップポインタ
	Material* materialData_ = nullptr;
	TransformationMatrix* wvpData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
};