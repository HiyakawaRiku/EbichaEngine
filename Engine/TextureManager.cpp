#include "TextureManager.h"
#include "DirectXCommon.h" // ConvertStringなどのヘルパー用
#include <cassert>
#include <Windows.h>

TextureManager* TextureManager::GetInstance() {
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {
    device_ = device;
    srvHeap_ = srvHeap;
    descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // テクスチャ配列の初期化
    textures_.clear();
}

// ★追加: メモリ上で1x1ピクセルの白色テクスチャを生成して 0番(kInvalidHandle) に登録する
void TextureManager::CreateWhiteTexture(ID3D12GraphicsCommandList* commandList) {
    DirectX::ScratchImage image{};
    // R8G8B8A8_UNORM フォーマットで 1x1 の画像を作成
    HRESULT hr = image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, 1);
    assert(SUCCEEDED(hr));

    // ピクセルデータ（RGBA = 255, 255, 255, 255 の白色）をセット
    const DirectX::Image* img = image.GetImage(0, 0, 0);
    uint8_t* pixels = img->pixels;
    pixels[0] = 255; // R
    pixels[1] = 255; // G
    pixels[2] = 255; // B
    pixels[3] = 255; // A

    const DirectX::TexMetadata& metadata = image.GetMetadata();

    // 1. リソース作成
    ID3D12Resource* textureResource = DirectXUtils::CreateTextureResource(device_, metadata);
    ID3D12Resource* intermediateResource = DirectXUtils::UploadTextureData(textureResource, image, device_, commandList);

    // 2. ディスクリプタハンドル（Index 0）
    TextureHandle handle = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvHeap_->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvHeap_->GetGPUDescriptorHandleForHeapStart();

    // 3. SRV生成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    device_->CreateShaderResourceView(textureResource, &srvDesc, cpuHandle);

    // 4. 管理データ登録
    TextureData data;
    data.resource.Attach(textureResource);
    data.cpuHandle = cpuHandle;
    data.gpuHandle = gpuHandle;

    textures_.push_back(data);
}

TextureHandle TextureManager::Load(const std::string& filePath, ID3D12GraphicsCommandList* commandList) {
    // 最初の読み込み時に1x1の白色テクスチャ（Index 0）を作っておく
    if (textures_.empty()) {
        CreateWhiteTexture(commandList);
    }

    // 読み込み済みなら既存のハンドルを返す
    auto it = textureMap_.find(filePath);
    if (it != textureMap_.end()) {
        return it->second;
    }

    assert(textures_.size() < kMaxTextureCount && "Exceeded maximum texture count.");

    // テクスチャの読み込み処理
    DirectX::ScratchImage image{};
    std::wstring filePathW = ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

    // ★ 修正点: 画像の読み込みに失敗した場合、落とさずに 0 番（1x1白色テクスチャ）を返す
    if (FAILED(hr)) {
        OutputDebugStringA(("[TextureManager] Failed to load: " + filePath + " -> Fallback to White Texture\n").c_str());
        textureMap_[filePath] = 0; // 次回も失敗したパスが来たら 0 を返す
        return 0;
    }

    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    if (FAILED(hr)) {
        mipImages = std::move(image); // ミップマップ生成に失敗した場合は原画像をそのまま使用
    }

    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    // 1. テクスチャリソース作成
    ID3D12Resource* textureResource = DirectXUtils::CreateTextureResource(device_, metadata);
    ID3D12Resource* intermediateResource = DirectXUtils::UploadTextureData(textureResource, mipImages, device_, commandList);

    // 2. ディスクリプタハンドルの算出
    TextureHandle handle = static_cast<TextureHandle>(textures_.size());

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvHeap_->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvHeap_->GetGPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += (descriptorSizeSRV_ * handle);
    gpuHandle.ptr += (descriptorSizeSRV_ * handle);

    // 3. SRVの生成
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    device_->CreateShaderResourceView(textureResource, &srvDesc, cpuHandle);

    // 4. 管理データへの登録
    TextureData data;
    data.resource.Attach(textureResource);
    data.cpuHandle = cpuHandle;
    data.gpuHandle = gpuHandle;

    textures_.push_back(data);
    textureMap_[filePath] = handle;

    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(TextureHandle handle) const {
    assert(handle < textures_.size());
    return textures_[handle].gpuHandle;
}