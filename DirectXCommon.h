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
	const char* blendMode_names = "None\0Normal\0Add\0Subtract\0Multiply\0Screen\0\0";

	BlendMode blendMode_ = BlendMode::kBlendModeScreen;
	void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; }

	static DirectXCommon* GetInstance();

	void Initialize();

	void PreDraw();

	void PostDraw();

	/// <summary>
	/// デバイスの取得
	/// </summary>
	/// <returns>デバイス</returns>
	ID3D12Device* GetDevice() const { return device_.Get(); }

	/// <summary>
	/// 描画コマンドリストの取得
	/// </summary>
	/// <returns>描画コマンドリスト</returns>
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	/// <summary>
	/// パイプライン生成
	/// </summary>
	void InitializePSO();

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
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_;
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resourcesForTransfer;
	UINT64 fenceVal_ = 0;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStates_[BlendMode::kCountOfBlendMode];
	//int32_t backBufferWidth_ = 0;
	//int32_t backBufferHeight_ = 0;
	//HANDLE frameLatencyWaitableObject_;
	//std::chrono::steady_clock::time_point reference_;
	//int32_t refreshRate_ = 0;

	// RTVを2つ作るのでデスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// SwapChainからResourceを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };
	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	HANDLE fenceEvent;
	// ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー矩形
	D3D12_RECT scissorRect{};

	static const uint32_t kMaxTextureIndex = 256;
public:
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU[kMaxTextureIndex];
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU[kMaxTextureIndex];

	void InitializeTexture(const std::string& filePath, uint32_t index);
private: // メンバ関数

	/// <summary>
	/// DXGIデバイス初期化
	/// </summary>
	void InitializeDXGIDevice();

	/// <summary>
	/// スワップチェーンの生成
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// コマンド関連初期化
	/// </summary>
	void InitializeCommand();

	/// <summary>
	/// レンダーターゲット生成
	/// </summary>
	void CreateFinalRenderTargets();

	/// <summary>
	/// フェンス生成
	/// </summary>
	void CreateFence();


	/// <summary>
	/// 
	/// </summary>
	void InitializeViewport();

	/// <summary>
	/// 
	/// </summary>
	void InitializeImgui();

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

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

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
