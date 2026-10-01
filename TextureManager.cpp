#include "TextureManager.h"
#include "EDirectX.h"
#include "Logger.h"

using namespace Logger;

void TextureManager::Initialize(EDirectX* eDirectX) {
    eDirectX_ = eDirectX;
}

void TextureManager::LoadTexture(const std::string& filePath, DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Resource>& textureResource, Microsoft::WRL::ComPtr<ID3D12Resource>& intermediateResource, DirectX::TexMetadata& metadata) {
    mipImages = ::LoadTexture(filePath);
    metadata = mipImages.GetMetadata();
    textureResource = eDirectX_->CreateTextureResource(metadata);
    intermediateResource = eDirectX_->UploadTextureData(textureResource, mipImages);
}

void TextureManager::CreateSRV(const DirectX::TexMetadata& metadata, Microsoft::WRL::ComPtr<ID3D12Resource> textureResource, uint32_t srvIndex, D3D12_CPU_DESCRIPTOR_HANDLE& srvHandleCPU, D3D12_GPU_DESCRIPTOR_HANDLE& srvHandleGPU) {
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    srvHandleCPU = eDirectX_->GetSRVCPUDescriptorHandle(srvIndex);
    srvHandleGPU = eDirectX_->GetSRVGPUDescriptorHandle(srvIndex);

    eDirectX_->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, srvHandleCPU);
}