#include "CommandContext.h"
#include <cassert>

CommandContext::~CommandContext() {
    if (fenceEvent_) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
}

void CommandContext::Initialize(ID3D12Device* device) {
    assert(device != nullptr);
    CreateCommandList(device);
    CreateFence(device);
}

void CommandContext::CreateCommandList(ID3D12Device* device) {
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    HRESULT hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    assert(SUCCEEDED(hr));
}

void CommandContext::CreateFence(ID3D12Device* device) {
    HRESULT hr = device->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));

    fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent_ != nullptr);
}

void CommandContext::Execute() {
    HRESULT hr = commandList_->Close();
    assert(SUCCEEDED(hr));

    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);
}

void CommandContext::WaitForGPU() {
    fenceVal_++;
    commandQueue_->Signal(fence_.Get(), fenceVal_);

    if (fence_->GetCompletedValue() < fenceVal_) {
        fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}

void CommandContext::Reset() {
    HRESULT hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));

    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr));
}