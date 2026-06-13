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

#include<dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include <wrl.h>

/// <summary>
/// DirectX汎用
/// </summary>
class DirectXCommon {
public: // メンバ関数
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static DirectXCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 描画後処理
	/// </summary>
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

private: // メンバ変数

	// Direct3D関連
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_;
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	//std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> resourcesForTransfer;
	UINT64 fenceVal_ = 0;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
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

	///// <summary>
	///// 深度バッファ生成
	///// </summary>
	//void CreateDepthBuffer();

	/// <summary>
	/// フェンス生成
	/// </summary>
	void CreateFence();

	/// <summary>
	/// パイプライン生成
	/// </summary>
	void InitializePSO();

	/// <summary>
	/// 
	/// </summary>
	void InitializeViewport();

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

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);