#include "SwapChainManager.h"
#include "DescriptorHeap.h"
#include "DirectXUtils.h"
#include <cassert>

void SwapChainManager::Initialize(
    IDXGIFactory7* dxgiFactory,
    ID3D12CommandQueue* commandQueue,
    ID3D12Device* device,
    DescriptorHeap* rtvHeap,
    DescriptorHeap* dsvHeap,
    HWND hwnd,
    uint32_t width,
    uint32_t height)
{
    assert(dxgiFactory && commandQueue && device && rtvHeap && dsvHeap);

    CreateSwapChain(dxgiFactory, commandQueue, hwnd, width, height);
    CreateRenderTargets(device, rtvHeap);
    CreateDepthStencil(device, dsvHeap, width, height);
}

void SwapChainManager::CreateSwapChain(IDXGIFactory7* dxgiFactory, ID3D12CommandQueue* commandQueue, HWND hwnd, uint32_t width, uint32_t height) {
    swapChainDesc_.Width = width;
    swapChainDesc_.Height = height;
    swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc_.SampleDesc.Count = 1;
    swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc_.BufferCount = kBackBufferCount;
    swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue,
        hwnd,
        &swapChainDesc_,
        nullptr, nullptr,
        &swapChain1);
    assert(SUCCEEDED(hr));

    hr = swapChain1.As(&swapChain_);
    assert(SUCCEEDED(hr));
}

void SwapChainManager::CreateRenderTargets(ID3D12Device* device, DescriptorHeap* rtvHeap) {
    rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (UINT i = 0; i < kBackBufferCount; ++i) {
        HRESULT hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
        assert(SUCCEEDED(hr));

        rtvHandles_[i] = rtvHeap->GetCPUDescriptorHandle(i);
        device->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc_, rtvHandles_[i]);
    }
}

void SwapChainManager::CreateDepthStencil(ID3D12Device* device, DescriptorHeap* dsvHeap, uint32_t width, uint32_t height) {
    depthStencilResource_ = DirectXUtils::CreateDepthStencilTextureResource(device, width, height);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    dsvHandle_ = dsvHeap->GetCPUDescriptorHandle(0);
    device->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvHandle_);
}

void SwapChainManager::PreDraw(ID3D12GraphicsCommandList* commandList) {
    UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

    // TransitionBarrier (PRESENT -> RENDER_TARGET)
    barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier_.Transition.pResource = swapChainResources_[backBufferIndex].Get();
    barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier_.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier_);

    // OMSetRenderTargets
    commandList->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, &dsvHandle_);

    // Clear
    float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void SwapChainManager::PostDraw(ID3D12GraphicsCommandList* commandList) {
    // TransitionBarrier (RENDER_TARGET -> PRESENT)
    barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    commandList->ResourceBarrier(1, &barrier_);
}

void SwapChainManager::Present(UINT syncInterval, UINT flags) {
    swapChain_->Present(syncInterval, flags);
}