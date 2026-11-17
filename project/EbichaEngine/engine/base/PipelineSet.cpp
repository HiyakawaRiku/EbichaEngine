#include "PipelineSet.h"
#include <cassert>

void PipelineSet::Create(ID3D12Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc) {
    for (int i = 0; i < (int)BlendMode::kCount; i++) {
        // ブレンド設定だけ差し替えてPSOを作る
        desc.BlendState = CreateBlendDesc((BlendMode)i);
        HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&psos_[i]));
        assert(SUCCEEDED(hr));
    }
}

ID3D12PipelineState* PipelineSet::Get(BlendMode mode) const {
    return psos_[(int)mode].Get();
}

void PipelineSet::Reset() {
    for (auto& pso : psos_) {
        pso.Reset();
    }
}