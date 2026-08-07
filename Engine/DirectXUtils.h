#pragma once
#include "DebugManager.h"

// DirectX12用
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <string>
#include <vector>
#include <wrl.h>

// libのリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include "DirectXTex.h"
#include "d3dx12.h"

namespace DirectXUtils {

	// 内部ユーティリティ（クラス内静的ヘルパー）
	 Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		Microsoft::WRL::ComPtr<IDxcUtils>& dxcUtils,
		Microsoft::WRL::ComPtr<IDxcCompiler3>& dxcCompiler,
		Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler);

	 ID3D12Resource* CreateBufferResource(
		const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);

	 ID3D12Resource* CreateTextureResource(
		ID3D12Device* device, const DirectX::TexMetadata& metadata);

	 ID3D12Resource* UploadTextureData(
		ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,
		ID3D12Device* device, ID3D12GraphicsCommandList* commandList);


	 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	 DirectX::ScratchImage LoadTexture(const std::string& filePath);

	 Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
		ID3D12Device* device, int32_t width, int32_t height);

	 D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	 D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
};