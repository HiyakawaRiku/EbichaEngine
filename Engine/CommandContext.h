#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class CommandContext {
public:
    CommandContext() = default;
    ~CommandContext();

    // 初期化 (CommandQueue, Allocator, List, Fenceの生成)
    void Initialize(ID3D12Device* device);

    // コマンドリストの確定・実行
    void Execute();

    // GPU同期処理 (Fenceシグナル・待機)
    void WaitForGPU();

    // 次フレーム用のコマンドリセット (Allocator / List のリセット)
    void Reset();

    // ゲッター
    ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

private:
    void CreateCommandList(ID3D12Device* device);
    void CreateFence(ID3D12Device* device);

private:
    // コマンドオブジェクト
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    // Fence同期オブジェクト
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    HANDLE fenceEvent_ = nullptr;
    UINT64 fenceVal_ = 0;
};