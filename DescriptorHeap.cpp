#include "DescriptorHeap.h"
#include <cassert>

void DescriptorHeap::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible) {
    assert(device != nullptr);

    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = type;
    desc.NumDescriptors = numDescriptors;
    desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_));
    assert(SUCCEEDED(hr));

    descriptorSize_ = device->GetDescriptorHandleIncrementSize(type);
    heapStartCPU_ = heap_->GetCPUDescriptorHandleForHeapStart();
    if (shaderVisible) {
        heapStartGPU_ = heap_->GetGPUDescriptorHandleForHeapStart();
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUDescriptorHandle(uint32_t index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = heapStartCPU_;
    handle.ptr += static_cast<size_t>(descriptorSize_) * index;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUDescriptorHandle(uint32_t index) const {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = heapStartGPU_;
    handle.ptr += static_cast<size_t>(descriptorSize_) * index;
    return handle;
}