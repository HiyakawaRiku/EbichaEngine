#pragma once
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include <wrl.h>
#include "DirectXTex.h"

// テクスチャ識別用のハンドル型
using TextureHandle = uint32_t;

class TextureManager {
public:
    static const TextureHandle kInvalidHandle = 0;
    static const uint32_t kMaxTextureCount = 256;

    static TextureManager* GetInstance();

    void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap);

    /// <summary>
    /// テクスチャファイルのロード（既に読込済みの場合はキャッシュされたハンドルを返す）
    /// </summary>
    TextureHandle Load(const std::string& filePath, ID3D12GraphicsCommandList* commandList);

    // ハンドルからGPUディスクリプタハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(TextureHandle handle) const;

private:
    TextureManager() = default;
    ~TextureManager() = default;

    ID3D12Device* device_ = nullptr;
    ID3D12DescriptorHeap* srvHeap_ = nullptr;
    UINT descriptorSizeSRV_ = 0;

    // ファイルパスとハンドルのマップ（重複読み込み防止）
    std::unordered_map<std::string, TextureHandle> textureMap_;

    // 生成されたテクスチャリソース群
    struct TextureData {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    };
    std::vector<TextureData> textures_;
};