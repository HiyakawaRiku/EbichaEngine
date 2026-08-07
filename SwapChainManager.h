#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>
#include <array>

class DescriptorHeap;

class SwapChainManager {
public:
    static constexpr uint32_t kBackBufferCount = 2;

    SwapChainManager() = default;
    ~SwapChainManager() = default;

    // 初期化 (スワップチェーン、RTV、DSVの生成)
    void Initialize(
        IDXGIFactory7* dxgiFactory,
        ID3D12CommandQueue* commandQueue,
        ID3D12Device* device,
        DescriptorHeap* rtvHeap,
        DescriptorHeap* dsvHeap,
        HWND hwnd,
        uint32_t width,
        uint32_t height);

    // 描画前処理 (PRESENT -> RENDER_TARGET バリア、RenderTarget/DSV設定、クリア)
    void PreDraw(ID3D12GraphicsCommandList* commandList);

    // 描画後処理 (RENDER_TARGET -> PRESENT バリア)
    void PostDraw(ID3D12GraphicsCommandList* commandList);

    // 画面フリップ (Present)
    void Present(UINT syncInterval = 1, UINT flags = 0);

    // ゲッター
    IDXGISwapChain4* GetSwapChain() const { return swapChain_.Get(); }
    DXGI_FORMAT GetRtvFormat() const { return rtvDesc_.Format; }
    UINT GetCurrentBackBufferIndex() const { return swapChain_->GetCurrentBackBufferIndex(); }
    ID3D12Resource* GetCurrentBackBuffer() const { return swapChainResources_[swapChain_->GetCurrentBackBufferIndex()].Get(); }

private:
    void CreateSwapChain(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue, HWND hwnd, uint32_t width, uint32_t height);
    void CreateRenderTargets(ID3D12Device* device, DescriptorHeap* rtvHeap);
    void CreateDepthStencil(ID3D12Device* device, DescriptorHeap* dsvHeap, uint32_t width, uint32_t height);

private:
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

    // RTV 関連
    Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[kBackBufferCount];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[kBackBufferCount]{};
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};

    // DSV 関連
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};

    // バリア構造体
    D3D12_RESOURCE_BARRIER barrier_{};
};