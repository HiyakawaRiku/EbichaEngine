#include "PipelineSet.h"
#include <cassert>

void PipelineSet::Create(ID3D12Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc) {
    // 呼び出し側が指定した深度書き込みの設定を覚えておく
    const D3D12_DEPTH_WRITE_MASK originalWriteMask = desc.DepthStencilState.DepthWriteMask;

    for (int i = 0; i < (int)BlendMode::kCount; i++) {
        BlendMode mode = (BlendMode)i;
        desc.BlendState = CreateBlendDesc(mode);

        // ブレンドなし → 元の設定のまま
        // ブレンドあり → 深度は「読むだけ・書かない」
        if (mode == BlendMode::kNone) {
            desc.DepthStencilState.DepthWriteMask = originalWriteMask;
        }
        else {
            desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        }

        HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&psos_[i]));
        assert(SUCCEEDED(hr));
    }
}

void PipelineSet::Reset() {
    for (auto& pso : psos_) {
        pso.Reset();
    }
	currentMode_ = BlendMode::kCount;
}

void PipelineSet::Apply(ID3D12GraphicsCommandList* commandList, BlendMode mode) {
    // 前回と同じモードなら、セットし直さない
    if (mode == currentMode_) {
        return;
    }
    currentMode_ = mode;
    commandList->SetPipelineState(psos_[(int)mode].Get());
}