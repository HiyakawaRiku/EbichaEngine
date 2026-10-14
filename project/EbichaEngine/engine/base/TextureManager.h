#pragma once
#include "DirectXCommon.h"
#include <vector>
#include <string>
#include <wrl.h>
#include <unordered_map>
#include "SrvManager.h"

class TextureManager
{
public:
	// テクスチャ 1 枚あたりのデータ構造体
	struct TextureData {
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

public:
	static TextureManager* GetInstance();

	// 初期化関数 (DirectXCommon を保持)
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	void Finalize();

	// テクスチャ読み込み関数 (読み込んだテクスチャのインデックスを返す)
	uint32_t Load(const std::string& filePath);

	// メタデータ取得
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

private:
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager = nullptr;
	std::unordered_map<std::string, TextureData> textureDatas;
	std::vector<std::string> textureKeys;
	//std::vector<TextureData> textureDatas_;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResources_;
};