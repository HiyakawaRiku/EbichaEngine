#pragma once
#include <string>
#include <wrl.h>
#include <d3d12.h>
#include "EDirectX.h"

class EDirectX;

class TextureManager {
public:
    TextureManager() = default;
    ~TextureManager() = default;

    // 初期化
    void Initialize(EDirectX* eDirectX);

    // テクスチャ読み込み＆リソース作成
    void LoadTexture(const std::string& filePath, DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Resource>& textureResource, Microsoft::WRL::ComPtr<ID3D12Resource>& intermediateResource, DirectX::TexMetadata& metadata);

    // SRV生成
    void CreateSRV(const DirectX::TexMetadata& metadata, Microsoft::WRL::ComPtr<ID3D12Resource> textureResource, uint32_t srvIndex, D3D12_CPU_DESCRIPTOR_HANDLE& srvHandleCPU, D3D12_GPU_DESCRIPTOR_HANDLE& srvHandleGPU);

private:
    EDirectX* eDirectX_ = nullptr;
};