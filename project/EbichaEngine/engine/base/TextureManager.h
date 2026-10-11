#pragma once
#include "DirectXCommon.h"
#include <vector>
#include <string>
#include <wrl.h>

class TextureManager
{
public:
	// テクスチャ 1 枚あたりのデータ構造体
	struct TextureData {
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

public:
	static TextureManager* GetInstance();

	// 初期化関数 (DirectXCommon を保持)
	void Initialize(DirectXCommon* dxCommon);
	void Finalize();

	// テクスチャ読み込み関数 (読み込んだテクスチャのインデックスを返す)
	uint32_t Load(const std::string& filePath);

	// メタデータ取得
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);

private:
	DirectXCommon* dxCommon_ = nullptr;
	std::vector<TextureData> textureDatas_;
};