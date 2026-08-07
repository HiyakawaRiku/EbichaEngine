#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <cstdint>
#include <array>
#include <string>

// ブレンドモード定義 (DirectXCommon.hからこちらに移動・共有しても構いません)
enum class BlendMode {
    kNone,
    kNormal,
    kAdd,
    kSubtract,
    kMultiply,
    kScreen,
    kCount,
};

class GraphicsPipelineManager {
public:
    GraphicsPipelineManager() = default;
    ~GraphicsPipelineManager() = default;

    // 初期化 (RootSignature および 各種 PSO の生成)
    void Initialize(ID3D12Device* device);

    // ゲッター
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState(BlendMode blendMode) const {
        return graphicsPipelineStates_[static_cast<size_t>(blendMode)].Get();
    }

private:
    void CreateRootSignature(ID3D12Device* device);
    void CreateGraphicsPipelines(ID3D12Device* device);
    D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode) const;

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCount)> graphicsPipelineStates_;
};