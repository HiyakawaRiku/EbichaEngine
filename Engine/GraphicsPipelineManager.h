#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <cstdint>
#include <array>
#include <string>

enum class PipelineType {
    kObject3D, // 通常描画用
    kParticle, // インスタンシング描画用
    kCount
};

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

enum class RootParameterIndex {
    kMaterial = 0,         // register(b0) : Pixel Shader
    kWVP = 1,              // register(b0) : Vertex Shader
    kTexture = 2,          // register(t0) : Descriptor Table
    kDirectionalLight = 3, // register(b1) : Pixel Shader
    kParticleInstance = 4, // register(t1) : StructuredBuffer (Vertex Shader)
    kCamera = 5,           // register(b2) : Pixel Shader / Vertex Shader
    kPointLight = 6,       // register(b3) : Pixel Shader ★追加
    kSpotLight = 7,        // register(b4) : Pixel Shader ★追加
    kCount
};

// 深度書き込み設定の追加
enum class DepthWrite {
    kEnable,  // D3D12_DEPTH_WRITE_MASK_ALL
    kDisable, // D3D12_DEPTH_WRITE_MASK_ZERO
    kCount
};

class GraphicsPipelineManager {
public:
    GraphicsPipelineManager() = default;
    ~GraphicsPipelineManager() = default;

    // 初期化 (RootSignature および 各種 PSO の生成)
    void Initialize(ID3D12Device* device);

    // ゲッター
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState(PipelineType pipelineType, BlendMode blendMode, DepthWrite depthWrite = DepthWrite::kEnable) const {
        return graphicsPipelineStates_[static_cast<size_t>(pipelineType)][static_cast<size_t>(blendMode)][static_cast<size_t>(depthWrite)].Get();
    }
    bool depthMask = false;

private:
    void CreateRootSignature(ID3D12Device* device);
    void CreateGraphicsPipelines(ID3D12Device* device);
    D3D12_BLEND_DESC CreateBlendDesc(BlendMode mode) const;


private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    std::array<
        std::array<
        std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(DepthWrite::kCount)>,
        static_cast<size_t>(BlendMode::kCount)
        >,
        static_cast<size_t>(PipelineType::kCount)
    > graphicsPipelineStates_;
};