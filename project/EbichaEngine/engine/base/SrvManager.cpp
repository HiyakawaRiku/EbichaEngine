//#include "SrvManager.h"
//#include "TextureManager.h"
//
//const uint32_t SrvManager::kMaxSRVCount = 128;
//
//void SrvManager::Initialize(DirectXCommon* dxCommon)
//{
//	this->dxCommon == dxCommon;
//
//	descriptorHeap = dxCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
//	descriptorSize = dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//}
//
//uint32_t SrvManager::Allocate()
//{
//	//assert(index<=useIndex);
//
//	int index = useIndex;
//	useIndex++;
//	return index;
//}
//
//void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
//{
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
//	DirectX::ScratchImage mipImages = dxCommon->LoadTexture(filePath);
//	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
//	srvDesc.Format = metadata.format;
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
//	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
//
//	dxCommon->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
//
//}
//
//void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride)
//{
//
//
//}
//
//
//D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
//{
//	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
//	handleCPU.ptr += (descriptorSize * index);
//	return handleCPU;
//
//}
//
//D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
//{
//	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
//	handleGPU.ptr += (descriptorSize * index);
//	return handleGPU;
//}
//
//
