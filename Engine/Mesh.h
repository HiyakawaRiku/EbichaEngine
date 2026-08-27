#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "EMath.h"
#include "Transform.h"
#include "Camera.h"

class Mesh {
public:
	// 初期化（頂点座標のセットとバッファ生成）
	void Initialize(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2);

	// 描画（トランスフォーム、カメラ、テクスチャを受け取って描画）
	void Draw(const Transform& transform, Camera* camera, TextureHandle textureHandle);

	// 色とライト設定のアクセサ（必要に応じて変更可能）
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	int32_t lightingType = 0; // None

private:
	void CreateVertexResource();
	void CreateVertexData(Vector4 vertex0, Vector4 vertex1, Vector4 vertex2);
	void CreateMaterialResource();
	void CreateWvpResource();
	void CreateDirectionalLight();

private:
	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();

	// 頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;

	// バッファのマップ先ポインタ
	Material* materialData_ = nullptr;
	TransformationMatrix* wvpData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
};