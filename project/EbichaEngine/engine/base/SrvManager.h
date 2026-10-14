#pragma once
#include "DirectXCommon.h"

class SrvManager
{
public:
	void Initialize(DirectXCommon* dxCommon);
	uint32_t Allocate();

	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride);

	void PreDraw();
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap.Get(); }

private:
	DirectXCommon* dxCommon = nullptr;

	static const uint32_t kMaxSRVCount;
	uint32_t descriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	uint32_t useIndex = 0;

};

