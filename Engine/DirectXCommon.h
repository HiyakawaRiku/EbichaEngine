#pragma once
#include "WinApp.h"
#include "DebugManager.h"
#include "DirectXUtils.h"
#include "DescriptorHeap.h"
#include "GraphicsPipelineManager.h"
#include "SwapChainManager.h"
#include "CommandContext.h"
#include <memory>
#include <string>

class DirectXCommon {
public:
	static const uint32_t kMaxTextureIndex = 256;
	static constexpr uint32_t kBackBufferCount = SwapChainManager::kBackBufferCount;

	static DirectXCommon* GetInstance();

	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;

	void Initialize();
	void Finalize();

	void PreDraw();
	void PostDraw();

	void InitializeTexture(const std::string& filePath, uint32_t index);

	// ゲッター / セッター
	ID3D12Device* GetDevice() const { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandContext_->GetCommandList(); }
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
	~DirectXCommon() = default;

	void InitializeDXGIDevice();
	void InitializeViewport();
	void InitializeImgui();

private:
	// 状態変数
	const char* blendModeNames_ = "None\0Normal\0Add\0Subtract\0Multiply\0Screen\0\0";
	BlendMode blendMode_ = BlendMode::kNone;

	WinApp* winApp_ = nullptr;

	// DXGI・デバイス
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	// サブシステムマネージャー群
	std::unique_ptr<DescriptorHeap> rtvHeap_;
	std::unique_ptr<DescriptorHeap> srvHeap_;
	std::unique_ptr<DescriptorHeap> dsvHeap_;
	std::unique_ptr<CommandContext> commandContext_;
	std::unique_ptr<SwapChainManager> swapChainManager_;
	std::unique_ptr<GraphicsPipelineManager> pipelineManager_;

	// テクスチャ用デスクリプタハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU[kMaxTextureIndex]{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU[kMaxTextureIndex]{};

	// ビューポート・シザー領域
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};
};