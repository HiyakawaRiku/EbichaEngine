#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class DescriptorHeap {
public:
    DescriptorHeap() = default;
    ~DescriptorHeap() = default;

    // 初期化
    void Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible);

    // ハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index = 0) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index = 0) const;

    // ゲッター
    ID3D12DescriptorHeap* GetHeap() const { return heap_.Get(); }
    uint32_t GetDescriptorSize() const { return descriptorSize_; }

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
    D3D12_CPU_DESCRIPTOR_HANDLE heapStartCPU_{};
    D3D12_GPU_DESCRIPTOR_HANDLE heapStartGPU_{};
    uint32_t descriptorSize_ = 0;
};