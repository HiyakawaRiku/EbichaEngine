#pragma once
#include "WinApp.h"

#include "DirectXUtils.h"
#include "DescriptorHeap.h"

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

	// シングルトンインスタンスの取得
	static DirectXCommon* GetInstance();

	// コピー・代入の禁止
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;

	// 初期化・終了処理
	void Initialize();
	void Finalize();

	// 描画前後処理
	void PreDraw();
	void PostDraw();

	// テクスチャ初期化
	void InitializeTexture(const std::string& filePath, uint32_t index);

	

	// ゲッター / セッター
	ID3D12Device* GetDevice() const { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
	//ID3D12DescriptorHeap* GetSrvHeap() const { return srvHeap_.Get(); }
	//ID3D12DescriptorHeap* GetRtvHeap() const { return rtvHeap_.Get(); }
	//ID3D12DescriptorHeap* GetDsvHeap() const { return dsvHeap_.Get(); }

	// ゲッターの更新 (既存コードとの互換性を維持)
	ID3D12DescriptorHeap* GetSrvHeap() const { return srvHeap_->GetHeap(); }
	ID3D12DescriptorHeap* GetRtvHeap() const { return rtvHeap_->GetHeap(); }
	ID3D12DescriptorHeap* GetDsvHeap() const { return dsvHeap_->GetHeap(); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureSrvHandleCPU(uint32_t index) const { return textureSrvHandleCPU[index - 1]; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU(uint32_t index) const { return textureSrvHandleGPU[index - 1]; }

	void SetBlendMode(BlendMode blendMode) { blendMode_ = blendMode; }
	BlendMode GetBlendMode() const { return blendMode_; }
	const char* GetBlendModeNames() const { return blendModeNames_; }

private:
	DirectXCommon() = default;
	~DirectXCommon();

	// 初期化サブメソッド
	void InitializeDXGIDevice();
	void CreateSwapChain();
	void InitializeCommand();
	void CreateFinalRenderTargets();
	void CreateFence();
	void InitializePSO();
	void InitializeViewport();
	void InitializeImgui();

	// PSO構築用の内部補助メソッド
	void CreateRootSignature();
	void CreateGraphicsPipelines();
	D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode) const;

	// 同期処理
	void WaitForGPU();

	

private:
	// 状態変数
	const char* blendModeNames_ = "None\0Normal\0Add\0Subtract\0Multiply\0Screen\0\0";
	BlendMode blendMode_ = BlendMode::kBlendModeNone;

	// アプリケーション参照
	WinApp* winApp_ = nullptr;

	// Direct3D12 基礎オブジェクト
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	// 記述子ヒープ
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;

	std::unique_ptr<DescriptorHeap> rtvHeap_;
	std::unique_ptr<DescriptorHeap> srvHeap_;
	std::unique_ptr<DescriptorHeap> dsvHeap_;

	// ディスクリプタ設定および保持構造体
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc_{};

	// レンダリングターゲット・リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[kBackBufferCount] = { nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[kBackBufferCount]{};
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;

	// テクスチャ用デスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU[kMaxTextureIndex]{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU[kMaxTextureIndex]{};

	// パイプライン・同期関連
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStates_[BlendMode::kCountOfBlendMode];

	D3D12_RESOURCE_BARRIER barrier_{};
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	HANDLE fenceEvent_ = nullptr;
	UINT64 fenceVal_ = 0;

	// ビューポート・シザー領域
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

};