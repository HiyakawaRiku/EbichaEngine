#include "TextureManager.h"
#include "DirectXCommon.h" // ConvertStringなどのヘルパー用
#include <cassert>

TextureManager* TextureManager::GetInstance() {
    static TextureManager instance;
    return &instance;
}

void TextureManager::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {
    device_ = device;
    srvHeap_ = srvHeap;
    descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // 0番は無効値用としてダミー領域を確保
    textures_.resize(1);
}

TextureHandle TextureManager::Load(const std::string& filePath, ID3D12GraphicsCommandList* commandList) {
    // 読み込み済みなら既存のハンドルを返す
    auto it = textureMap_.find(filePath);
    if (it != textureMap_.end()) {
        return it->second;
    }

    assert(textures_.size() < kMaxTextureCount && "Exceeded maximum texture count.");

    // テクスチャの読み込み処理 (既存の DirectXCommon 内にあったもの)
    DirectX::ScratchImage image{};
    std::wstring filePathW = ConvertString(filePath);
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));

    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

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