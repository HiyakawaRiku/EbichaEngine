#pragma once
#include "WinApp.h"
#include "DebugManager.h"

// DirectX12用
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
// libのリンク
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

#include<dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#include <wrl.h>

#include "Input.h"

enum BlendMode {
	kBlendModeNone,
	kBlendModeNormal,
	kBlendModeAdd,
	kBlendModeSubtract,
	kBlendModeMultiply,
	kBlendModeScreen,
	kCountOfBlendMode,
};


class DirectXCommon {
public: // メンバ関数
	static const uint32_t kMaxTextureIndex = 256;
	static constexpr uint32_t kBackBufferCount = 2;

	const char* blendMode_names = "None\0Normal\0Add\0Subtract\0Multiply\0Screen\0\0";
	BlendMode blendMode_ = BlendMode::kBlendModeNone;


	static DirectXCommon* GetInstance();

	void Initialize();
	//void Finalize(); // 後処理用の終了関数

	void PreDraw();
	void PostDraw();
	
	ID3D12Device* GetDevice() const { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; }
	void InitializeTexture(const std::string& filePath, uint32_t index);

public:
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU[kMaxTextureIndex];
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU[kMaxTextureIndex];

private: // メンバ関数

	void InitializeDXGIDevice();
	void CreateSwapChain();
	void InitializeCommand();
	void CreateFinalRenderTargets();
	void CreateFence();
	void InitializePSO();
	void InitializeViewport();
	void InitializeImgui();

private: // メンバ変数
	// ウィンドウズアプリケーション管理
	WinApp* winApp_ = WinApp::GetInstance();

	// Direct3D関連
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStates_[BlendMode::kCountOfBlendMode];

	D3D12_RESOURCE_BARRIER barrier{};
	HANDLE fenceEvent;
	UINT64 fenceVal_ = 0;

	D3D12_VIEWPORT viewport{};
	D3D12_RECT scissorRect{};

	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[kBackBufferCount] = { nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[kBackBufferCount];



};

Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
	// CompilerするShaderファイルへのパス
	const std::wstring& filePath,
	// Compilerに使用するProfile
	const wchar_t* profile,
	// 初期化で生成したものを3つ
	Microsoft::WRL::ComPtr<IDxcUtils>& dxcUtils,
	Microsoft::WRL::ComPtr<IDxcCompiler3>& dxcCompiler,
	Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler);

ID3D12Resource* CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);
ID3D12DescriptorHeap* CreateDescriptorHeap(
	ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
DirectX::ScratchImage LoadTexture(const std::string& filePath);
ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

[[nodiscard]]
inline ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(),
		subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
	ID3D12Resource* intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresources.size()), subresources.data());
	// Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}


inline D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

inline D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}
